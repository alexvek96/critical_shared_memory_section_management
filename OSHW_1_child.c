// OS Assignment 1
// child code
#include "declarations.h"

int main(void){

    int shmid, semid1, semid2, semid3;
    int i, j, segment, line;
    float possibility;                  // between 0 and 1
    void *shared_memory = (void *)0;
    struct sembuf sem1, sem2, sem3;
    struct shared_mem *mem;
    long long milliseconds;
    FILE* log;
    srand(getpid());

    // child obtains the existing shared memory
    if ((shmid = shmget(SHMKEY, sizeof(struct shared_mem), 0666 | IPC_CREAT)) == -1) {

		printf("Error! Child could not obtain the shared memory [shmget()].\n");
		shmctl(shmid, IPC_RMID, NULL);
		exit(1);
	}

    // obtain the semaphores
    if ((semid1 = semget(SEMKEY1, 0, 0)) == -1) {

		printf("Error! Child could not obtain semaphore with semget().\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);     // free all
		exit(1);
	}
    if ((semid2 = semget(SEMKEY2, 0, 0)) == -1) {

		printf("Error! Child could not obtain semaphore with semget().\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);     // free all
        semctl(semid2, 0, IPC_RMID, 0);
		exit(1);
	}
    if ((semid3 = semget(SEMKEY3, 0, 0)) == -1) { 

		printf("Error! Child could not obtain semaphore with semget().\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);     // free all
        semctl(semid2, 0, IPC_RMID, 0);
        semctl(semid3, 0, IPC_RMID, 0);
		exit(1);
	}

    // attach shared memory to child

    if ((shared_memory =  shmat(shmid, NULL, 0))== (void *) -1) {

		printf("Error! Unable to attach shared memory to child.\n");
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid1, 0, IPC_RMID, 0);
        semctl(semid2, 0, IPC_RMID, 0);
        semctl(semid3, 0, IPC_RMID, 0);
		exit(1);

    } else {

		printf("Shared memory with id '%d' attached to child #%d.\n", shmid, getpid());
	}

    mem = (struct shared_mem *) shared_memory;
    
    // creation of log file for each child
    char filename[35] = "logfile_of_child_#";
    char buf2[10];
    sprintf(buf2, "%d", getpid());
    char* buf3 = ".txt";
    strcat(filename, buf2);
    strcat(filename, buf3);

    // open log file to write child's request specs
    log = fopen(filename, "a");

    if (log==NULL){

        printf("Error opening log file!\n");
    }

    for (i=1; i<=(mem->num_of_requests); i++){

        (mem->count)++; // one more request in total

        if (mem->count > 1){

            mem->prev_segment = mem->req_segment;   // keeping history
            possibility = (float) rand()/RAND_MAX;
            if (possibility <= 0.3){

                segment = (rand() % (mem->num_of_segments));
            }
            else{

                segment = mem->prev_segment;
            }
        }else if (mem->count == 1){

            segment = (rand() % (mem->num_of_segments));
        }

        line = (rand() % (mem->lines_per_segment));

        time_t begin;
        time(&begin);        
        char* t1 = ctime(&begin); 

        for (j=1; j<=5; j++){
            t1[strlen(t1) - 1] = '\0';
        }

        // if segment is already on the shared memory, child enters and gets it
        if ((segment + 1) == mem->prev_segment){

            printf("Child #%d to parent: acquired line %d of segment %d.\n", getpid(), line+1, segment+1);
            sleep(0.02);    // data editing simulation
            time_t end;

            //datetime string operations
            milliseconds = find_milliseconds();
            time(&end);
            char* t2 = ctime(&end);

            for (j=1; j<=6; j++){
                t2[strlen(t2) - 1] = '\0';
            }

            fprintf(log, "Request #%d <segment, line>: <%d,%d>\n", i, segment+1, line+1);   
            // because of '%' in lines 127-128 and printing purposes -> segment+1, line+1
            
            if (milliseconds > 99){     // example:  87 -> '087'

                fprintf(log, "  Request time: %s.%lld  %d\n", t1, milliseconds, YEAR);
                fprintf(log, "  Serve time:   %s.%lld  %d\n", t2, milliseconds, YEAR);
                fprintf(log, "  Line: %s", mem->text_seg[line]);
                fprintf(log, "---------------------------------------------------------------------------------------------------------\n");
            
            }else{                      // example:  874 -> '874'
                
                fprintf(log, "  Request time: %s.0%lld  %d\n", t1, milliseconds, YEAR);
                fprintf(log, "  Serve time:   %s.0%lld  %d\n", t2, milliseconds, YEAR);
                fprintf(log, "  Line: %s", mem->text_seg[line]);
                fprintf(log, "---------------------------------------------------------------------------------------------------------\n");
            }
        }
        else{
            // wanted segment is not in the shared memory, child goes and requests it
            sem1.sem_num = 0;
            sem1.sem_op = -1;
            sem1.sem_flg = 0;

            // child tries to lock the semaphore and enter the critical section
            
            if (semop(semid1, &sem1, lock) < 0) {
                
                printf("Error with semop().\n");
                shmctl(shmid, IPC_RMID, NULL);
                semctl(semid1, 0, IPC_RMID, 0);
                exit(1);
            }

            // request section

            printf("Child #%d to parent: requesting line %d of segment %d.\n", getpid(), line+1, segment+1);
            // because of '%' in lines 127-128 and printing purposes -> segment+1, line+1

            // update shm data for the parent to get the request info
            mem->chpid = getpid();
            mem->req_segment = segment+1;
            mem->line_of_segment = line+1;

            sem2.sem_num = 0;
            sem2.sem_op = 1;
            sem2.sem_flg = 0;
            
            // child unlocks the parent so he can get the request info
            if (semop(semid2, &sem2, unlock) < 0){	

                printf("Error with semop().\n");
                shmctl(shmid, IPC_RMID, NULL);
                semctl(semid1, 0, IPC_RMID, 0);
                semctl(semid2, 0, IPC_RMID, 0);
                semctl(semid3, 0, IPC_RMID, 0);
                exit(1);
            }

            sem2.sem_num = 0;
            sem2.sem_op = -1;
            sem2.sem_flg = 0;
            sleep(1);

            // child locks sem2 for parent's NEXT loop run
            if (semop(semid2, &sem2, lock) < 0){

                printf("Error with semop().\n");
                shmctl(shmid, IPC_RMID, NULL);
                semctl(semid1, 0, IPC_RMID, 0);
                semctl(semid2, 0, IPC_RMID, 0);
                semctl(semid3, 0, IPC_RMID, 0);
                exit(1);
            }

            sem3.sem_num = 0;
            sem3.sem_op = -1;
            sem3.sem_flg = 0;

            // child tries to lock sem3 and enter to get parent's answer
            if (semop(semid3, &sem3, lock) < 0){

                printf("Error with semop().\n");
                shmctl(shmid, IPC_RMID, NULL);
                semctl(semid1, 0, IPC_RMID, 0);
                semctl(semid2, 0, IPC_RMID, 0);
                semctl(semid3, 0, IPC_RMID, 0);
                exit(1);
            }

            printf("Child #%d to parent: acquired line %d of segment %d.\n", getpid(), line+1, segment+1);
            // because of '%' in lines 127-128 and printing purposes -> segment+1, line+1

            time_t end2;
            milliseconds = find_milliseconds();
            time(&end2);

            //datetime string operations
            char* t3 = ctime(&end2);
            for (j=1; j<=6; j++){
                t3[strlen(t3) - 1] = '\0';
            }

            // data editing simulation
            sleep(0.02);    
            fprintf(log, "Request #%d <segment, line>: <%d,%d>\n", i, segment+1, line+1);
            // because of '%' in lines 127-128 and printing purposes -> segment+1, line+1
            
            if (milliseconds > 99){  

                // example:  874 -> '874'
                fprintf(log, "  Request time: %s.%lld  %d\n", t1, milliseconds, YEAR);
                fprintf(log, "  Serve time:   %s.%lld  %d\n", t3, milliseconds, YEAR);
                fprintf(log, "  Line: %s", mem->text_seg[line]);
                fprintf(log, "---------------------------------------------------------------------------------------------------------\n");
            
            }else{     

                // example:  87 -> '087'
                fprintf(log, "  Request time: %s.0%lld  %d\n", t1, milliseconds, YEAR);
                fprintf(log, "  Serve time:   %s.0%lld  %d\n", t3, milliseconds, YEAR);
                fprintf(log, "  Line: %s", mem->text_seg[line]);
                fprintf(log, "---------------------------------------------------------------------------------------------------------\n");

            }            
            // child finished request
            sem3.sem_num = 0;
            sem3.sem_op = 1;
            sem3.sem_flg = 0;

            // child unlocks sem3 for other children request section
            if (semop(semid3, &sem3, unlock) < 0){

                printf("Error with semop().\n");
                shmctl(shmid, IPC_RMID, NULL);
                semctl(semid1, 0, IPC_RMID, 0);
                semctl(semid2, 0, IPC_RMID, 0);
                semctl(semid3, 0, IPC_RMID, 0);
                exit(1);
            }

            sem1.sem_num = 0;
            sem1.sem_op = 1;
            sem1.sem_flg = 0;
            
            // child unlocks semaphore for other children to enter and request
            if (semop(semid1, &sem1, unlock) < 0) {	

                printf("Error with semop().\n");
                shmctl(shmid, IPC_RMID, NULL);
                semctl(semid1, 0, IPC_RMID, 0);     // free all
                semctl(semid2, 0, IPC_RMID, 0);
                semctl(semid3, 0, IPC_RMID, 0);
                exit(1);
            }

        }
    }

    // child finished ALL requests

    fclose(log);
    printf("Child #%d finished all requests.\n", getpid());                                    
    // detach shared memory from child
    shmdt(shared_memory);
    exit(0);

}