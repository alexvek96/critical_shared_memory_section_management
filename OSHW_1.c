// OS Assignment 1
// parent code
#include "declarations.h"

int main(void){

    int segment_size, number_of_segments, requests, i, j;
    int shmid, pid, status;
    int semid1, semid2, semid3;
    int directory_length, num_of_children;
    // maximum hypothetical length of any line of the text file
    // it is too long on purpose, usually lines are much shorter
    int max_possible_length, k;
    int num_of_lines = 1;
    char c, client_exe[255];
    void *shared_memory = (void *)0;
    struct sembuf sem1, sem2, sem3;
    struct shared_mem *mem;
    long long milliseconds;
    FILE* text_file; 
    FILE* log;

    printf("File name we use: 'ostext.txt' \n");
    if ((text_file = fopen("ostext.txt", "r")) == NULL){        // text file we will use
        printf("Error! Unable to open file.");
        exit(1);
    }
    remove("logfile_of_PARENT.txt");    // removes previous log files, if any (for more than 1 executions of the program)

    // user gives the segment size (number of lines in each division) as input
    // we also check they give a reasonable size number, we assume >=10 and <=150
    printf("Ready to divide the text lines into segments.\n");
    printf("Each segment can contain between 10 and 150 lines.\n");
    printf("Please type the number of lines you wish each segment to contain: ");
    scanf("%d", &segment_size);
    printf("Divided the text into segments of %d lines at most. \n", segment_size);

    
    while ((segment_size < 10) || (segment_size > 150)){

        printf("Error! You gave a wrong number. Try again.\n");
        printf("Ready to divide the text lines into segments.\n");
        printf("Each segment can contain between 10 and 150 lines.\n");
        printf("Please type the number of lines you wish each segment to contain: ");
        scanf("%d", &segment_size);
    }

    // user gives the number of children he wants to be created by the parent
    printf("Please type the number of childen you want the parent proccess to create: ");
    scanf("%d", &num_of_children);

    // user gives the number of requests he wants each child to send to the mother proccess
    printf("Please type the number of requests you want each child to send to the parent proccess: ");
    scanf("%d", &requests);

    max_possible_length = 0;

    k = 0;  // 'k' counts th length of each line in pure characters without '\n'
    while ((c=getc(text_file)) != EOF){

        k++;
        if (c == '\n'){
            if (k > max_possible_length){
                max_possible_length = k;        // update the maximum possible length of a line
                k = 0;
            }
            num_of_lines++;
        }
    }
    fclose(text_file);

    // creating a matrix to store the whole text as characters in cells
    char* text[num_of_lines];
    if ((text_file = fopen("ostext.txt", "r")) == NULL){        // text file we will use
        
        printf("Error! Unable to open file.");
        exit(1);
    }
    char line[max_possible_length+1];

    // copying text lines into a helper-array
    i=0;
    while ((fgets(line, sizeof(line), text_file)) != NULL){

        text[i] = strdup(line);
        i++;
    }

    // creation of the shared memory
    if ((shmid = shmget(SHMKEY, sizeof(struct shared_mem), 0666 | IPC_CREAT)) == -1) {

		printf("Error! Could not create the shared memory.\n");
		shmctl(shmid, IPC_RMID, NULL);
		exit(1);
	}

    printf("Created shared memory with ID: %d\n", shmid);
    
    // attachment of the shared memory
    shared_memory = shmat(shmid, (void *)0, 0);

    if (shared_memory == (void *)-1) {	

		printf("Error! Unable to attach the shared memory to the parent.\n");
		shmctl(shmid, IPC_RMID, NULL);
		exit(1);
    }

	printf("Shared memory has been attached to the parent.\n");
    mem = (struct shared_mem *) shared_memory;
    mem->num_of_requests = requests;
    mem->lines_per_segment = segment_size;
    number_of_segments = (int) (num_of_lines / segment_size);
    mem->lines_of_last_segment = num_of_lines % number_of_segments;
    mem->num_of_segments = number_of_segments;
    mem->count = 0;
    mem->prev_segment = -1;
    // -1 due to initialization purposes - GENESIS segment
    // now, allocate memory for shared text segment

    // acquire current working path, so we can run the child's main code
    getcwd(client_exe, 255);
    directory_length = strlen(client_exe);
    strcpy(client_exe + directory_length, "/OSHW_1_child");

    printf("Initialization of the semaphores...\n");
    // create the semaphores
    // 1
    if ((semid1 = semget(SEMKEY1, 1, IPC_CREAT | IPC_EXCL | 0600)) == -1) {

		printf("Error with semget().\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);         // free all allocations if there is a fatal error
        exit(1);
	}
    printf("Created semaphore '1' with ID: %d \n", semid1);

    // 2
    if ((semid2 = semget(SEMKEY2, 1, IPC_CREAT | IPC_EXCL | 0600)) == -1) {

		printf("Error with semget().\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);         // free all allocations if there is a fatal error
		semctl(semid2, 0, IPC_RMID, 0);
        exit(1);
	}
    printf("Created semaphore '2' with ID: %d \n", semid2);

    // 3
    if ((semid3 = semget(SEMKEY3, 1, IPC_CREAT | IPC_EXCL | 0600)) == -1) {

        printf("%d\n", semid3);			
		printf("Error with semget().\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);         // free all allocations if there is a fatal error
		semctl(semid2, 0, IPC_RMID, 0);
        semctl(semid3, 0, IPC_RMID, 0);
        exit(1);
	}
    printf("Created semaphore '3' with ID: %d \n", semid3);

    // set the value of the rest semaphores to 0 to locked
    // 1
    if (semctl(semid1, 0, SETVAL, 0) == -1) {

		printf("Error with semctl().\n");
		shmctl(shmid,IPC_RMID,NULL);
		semctl(semid1, 0, IPC_RMID, 0);
		semctl(semid2, 0, IPC_RMID, 0);         // free all allocations if there is a fatal error
        semctl(semid3, 0, IPC_RMID, 0);
		exit(1);
	}

    // 2
    if (semctl(semid2, 0, SETVAL, 0) == -1) {

		printf("Error with semctl().\n");
		shmctl(shmid,IPC_RMID,NULL);
		semctl(semid1, 0, IPC_RMID, 0);
		semctl(semid2, 0, IPC_RMID, 0);         // free all allocations if there is a fatal error
        semctl(semid3, 0, IPC_RMID, 0);
		exit(1);
	}

    // 3
    if (semctl(semid3, 0, SETVAL, 0) == -1) {

		printf("Error with semctl().\n");
		shmctl(shmid,IPC_RMID,NULL);
		semctl(semid1, 0, IPC_RMID, 0);
		semctl(semid2, 0, IPC_RMID, 0);         // free all allocations if there is a fatal error
        semctl(semid3, 0, IPC_RMID, 0);
		exit(1);
	}	

    // creation of children
    for(i=1; i<=num_of_children; i++){

		if ((pid = fork()) == -1) {	

            printf("Error! Could not fork and create the child #%d.\n", i);
			shmctl(shmid, IPC_RMID, NULL);
			semctl(semid1, 0, IPC_RMID, 0);
			semctl(semid2, 0, IPC_RMID, 0);     // free all allocations if there is a fatal error
            semctl(semid3, 0, IPC_RMID, 0);
			exit(1);
        }
        else if (pid == 0) {

            printf("Child #%d created.\n", getpid());
            if (i==num_of_children){
                printf("Creation of all children finished.\n");
            }
            // if it is the child, execute its code in the 'OSHW_1_child.c' file						
            execl(client_exe, "OSHW_1_child", NULL);
            _exit(127);
        }
	}
    
    sem1.sem_num = 0;
    sem1.sem_op = 1;
    sem1.sem_flg = 0;

    // unlock the semaphore for children to entry their requests loop
    if (semop(semid1, &sem1, unlock) < 0) {	

        printf("Error with semop().\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);
        exit(1);
    }

    // creation of log file for the parent
    char filename[35] = "logfile_of_PARENT";
    char* buf2 = ".txt";
    strcat(filename, buf2);

    // open log file to write child's request specs
    log = fopen(filename, "a");

    // requests serving

    for (k=1; k<=(num_of_children*requests); k++){

        sem2.sem_num = 0;
        sem2.sem_op = -1;
        sem2.sem_flg = 0;

        // parent tries to lock the semaphore to enter and start serving children's requests
        if (semop(semid2, &sem2, lock) < 0) {

            printf("Error with semop().\n");
            shmctl(shmid, IPC_RMID, NULL);
            semctl(semid1, 0, IPC_RMID, 0);
            semctl(semid2, 0, IPC_RMID, 0);
            semctl(semid3, 0, IPC_RMID, 0);
            exit(1);
        }
        
        // counting starts from 0, so line 1 is the text[0]
        if (number_of_segments != mem->req_segment){

            // because of '%' in lines 127-128 and printing purposes -> segment+1, line+1
            for (i=0; i<segment_size; i++){
                
                // first, re-initialize each array line, so we can clearly copy each text line
                for (j=0; j<200; j++){
                    mem->text_seg[i][j] = '\0';
                }
                strcpy(mem->text_seg[i], text[((mem->req_segment)-1)*segment_size + i]);
            }
            
            time_t upload;
            milliseconds = find_milliseconds();
            time(&upload);

            //datetime string operations
            char* s1 = ctime(&upload);
            for (j=1; j<=6; j++){
                s1[strlen(s1) - 1] = '\0';
            }

            if (milliseconds > 99){  

                // example:  875 -> '875'
                fprintf(log, "PARENT: Upload time at #%d upload:       %s.%lld %d\n", k, s1, milliseconds, YEAR);
            
            }else{ 

                // example:  87 -> '087'
                fprintf(log, "PARENT: Upload time at #%d upload:       %s.0%lld %d\n", k, s1, milliseconds, YEAR);
            }

            if (k == 1){

                time_t download;
                milliseconds = find_milliseconds();
                time(&download);

                //datetime string operations
                char* s2 = ctime(&download);

                for (j=1; j<=6; j++){
                    s2[strlen(s2) - 1] = '\0';
                }

                if (milliseconds > 99){
                    // example:  875 -> '875'
                    fprintf(log, "PARENT: Withdrawed GENESIS segment at:  %s.%lld %d\n", s2, milliseconds, YEAR);
                    fprintf(log, "---------------------------------------------------------------------------------------------------------\n");
                }else{
                    // example:  87 -> '087'
                    fprintf(log, "PARENT: Withdrawed GENESIS segment at:  %s.0%lld %d\n", s2, milliseconds, YEAR);
                    fprintf(log, "---------------------------------------------------------------------------------------------------------\n");

                }

            } else if (k > 1){

                time_t download;
                milliseconds = find_milliseconds();
                time(&download);

                //datetime string operations
                char* s3 = ctime(&download);
                for (j=1; j<=6; j++){
                    s3[strlen(s3) - 1] = '\0';
                }

                if (milliseconds > 99){
                    // example:  875 -> '875'
                    fprintf(log, "PARENT: Withdrawal time at #%d withdraw: %s.%lld %d\n", k, s3, milliseconds, YEAR);
                    fprintf(log, "---------------------------------------------------------------------------------------------------------\n");
                }else{
                    // example:  87 -> '087'
                    fprintf(log, "PARENT: Withdrawal time at #%d withdraw: %s.0%lld %d\n", k, s3, milliseconds, YEAR);
                    fprintf(log, "---------------------------------------------------------------------------------------------------------\n");
                }
            }       
        }
        else if ((mem->line_of_segment) > (mem->lines_of_last_segment)){

            // if the child has asked something from the last segment, which may have less lines than the other segments
            // if it asks for a line that exceeds the already existing lines 
            printf("Parent to child #%d: You asked for segment %d and line %d.\n", mem->chpid, mem->req_segment, mem->line_of_segment);
            // because of '%' in lines 127-128 and printing purposes -> segment+1, line+1
            printf("Parent to child #%d: Error! The line and segment you are requesting does not exist.\n", mem->chpid);
        }
        else{

            // if the child has asked something from the last segment, which may have less lines than the other segments
            // and that line exists in the last segment
            printf("Parent to child #%d: You asked for segment %d and line %d. Uploading your request...\n", mem->chpid, mem->req_segment, mem->line_of_segment);
            // because of '%' in lines 127-128 and printing purposes -> segment+1, line+1
            
            for (i=0; i<mem->lines_of_last_segment; i++){

                strcpy(mem->text_seg[i], text[num_of_lines - mem->lines_of_last_segment + i]);
            }
            
        }

        sem2.sem_num = 0;
        sem2.sem_op = 1;
        sem2.sem_flg = 0;

        // parent instantly unlocks the semaphore so the child can lock it for next parent's run
        if (semop(semid2, &sem2, unlock) < 0) {

            printf("Error with semop().\n");
            shmctl(shmid, IPC_RMID, NULL);
            semctl(semid1, 0, IPC_RMID, 0);
            semctl(semid2, 0, IPC_RMID, 0);
            semctl(semid3, 0, IPC_RMID, 0);
            exit(1);
        }
        
        sem3.sem_num = 0;
        sem3.sem_op = 1;
        sem3.sem_flg = 0;

        // after he uploads its answer, parent unlocks sem3 for the child to get its answer
        if (semop(semid3, &sem3, unlock) < 0){	

            printf("Error with semop().\n");
            shmctl(shmid, IPC_RMID, NULL);
            semctl(semid1, 0, IPC_RMID, 0);
            semctl(semid2, 0, IPC_RMID, 0);
            semctl(semid3, 0, IPC_RMID, 0);
            exit(1);
        }
        sleep(1);
        
    }

    // close the log file of parent
    fclose(log);

    // section of waiting all children to exit

    for(;;){

        // pick each finishing child's pid
		pid = wait(&status);            
		if (pid < 0) {	
            if (errno == ECHILD) {
                printf("All children have exited.\n");   // if all the children have exited
                break;                                   // break the loop and finish the parent     
            }
            else {
                printf("Error! Problem occured with wait().\n");
            }
        }
        else {
            printf("Child #%d exited with status '%d'.\n", pid, status);
        }
	}

	shmdt(shared_memory);				// detach shared memory	
	shmctl(shmid, IPC_RMID, NULL);		// remove shared memory	
	semctl(semid1, 0, IPC_RMID, 0);	    // remove semaphores
	semctl(semid2, 0, IPC_RMID, 0);
    semctl(semid3, 0, IPC_RMID, 0);
	exit(0);
}