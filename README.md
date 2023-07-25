# critical_shared_memory_section_management
1st/2 Assignment of the "Operating Systems" course (Winter Semester 2022/2023 - NKUA). Use of semaphores to control parent and children processes access to a critical memory section. The code implements the server-client model (clients ask for some data and the data is served back). More info in the README file...

**File Description**
--------------------

The project files are five (5):

1) declarations.h -> Contains all includes, defines, the definition of the struct "shared memory," and
    a helper function "find_milliseconds()." Briefly, it's worth mentioning that the definition of the variables 
	"lock" and "unlock" aims only to enhance the visual understanding of the code during the ups and downs of the 
	semaphores. The "find_milliseconds()" function returns the milliseconds of the current timestamp. Within the 
	shared memory, there is a character array "text_seg[150][200]," which is essentially used to store each segment 
	of the total text file. The dimensions of the array were initially set in such a way that they would be sufficient 
	(and more) for very large texts and line lengths. The dimensions are derived from the condition that the text can 
	be divided into segments of at least 10 to 150 lines each (considered a sufficient condition for the project). 
	The user selects the option from the keyboard with simultaneous validation of incorrect input. The remaining 
	variables in the shared memory are either explained with comments or their usage is evident from their names.

2) ostext.txt -> This is the txt file processed by the main code. It contains a total of 1251 lines.

3) Makefile (the compile and run commands are provided at the end of the README)

4) OSHW_1.c -> Includes the code for the parent process. Initially, there is a set of outputs to the user, through which 
the user inputs data, such as the segment size (= number of lines each segment should have), the number of children the 
parent wants to create, and the number of requests each child should make. The last two values do not have a value restriction, 
but for large numbers, the program will obviously take longer to execute. The parent creates its own log file to record the 
upload and download times for each segment. The project was implemented using three (3) SysV semaphores and one (1) shared 
memory. Some data initialization is done by the parent in certain fields of the struct, which is explained in the corresponding 
line of code with a comment. Also, effort was made to interact with the user and display the execution states and results in detail. 
In the parent's code, at the points where times and data related to interactions with the children are printed, some string 
manipulations are performed (and certain numerical conditions are checked) solely for better and correct data representation 
in the parent's log file. An important initialization worth mentioning is the initial value of the requested segment (int req_segment = -1). 
The value -1 was set to help in starting the requests from the children and in some specific printouts since there is no request history yet. 
The first segment withdrawn by the parent is randomly named "GENESIS segment" and marks the start of the normal request-serve process. 
Additionally, apart from the parent's log file, messages are printed in the terminal to represent the communication between the parent 
and children (the child requests something, the parent serves it, and the child confirms the receipt for us to proceed to the next steps). 
Finally, the parent waits until all children finish their tasks.

5) OSHW_1_child.c -> Contains the code for the children. After the necessary semaphore obtains and shared memory attachment, each 
child creates its own log file (one for each child) to print the project-related requests concerning the children. Similarly, effort 
was made to interact with the user and display the execution states and results in detail. In the parent's code, at the points where 
times and data related to interactions with the children are printed, some string manipulations are performed (and certain numerical 
conditions are checked) solely for better and correct data representation in each child's log file.

**Semaphores**
--------------

The project was implemented using three (3) semaphores (sem1, sem2, sem3). Initially, sem1 is unlocked by the parent to allow the 
fastest child to enter and start the scenario. From then on, the parent no longer has authority over sem1, and it is used by the 
children to control each other's access (the fastest child enters, etc.). Sem2 and sem3 are used for synchronization and access 
to shared memory between parent and child. Initially, the parent waits for the child to make its request and leave its information 
in the fields of the shared memory, then the parent enters and locks the child (sem2). After fulfilling the request with the 
corresponding upload, it unlocks the child again for the child to acquire the information (sem3). In the meantime, the parent waits, 
locked (the child has already entered and locked sem2), and waits for another child to enter and repeat the same process for the 
next child or request from the same child. This happens as soon as another child is unlocked through sem1 and makes its request, 
then unlocks sem2 for the parent, etc. The demand for no mutual exclusion when more than one (1) child desires the same uploaded 
and shared segment is implemented with a simple if statement. If the child's requested segment is already uploaded in the shared 
memory, it simply retrieves the line it wants, so it does not exclude other children from the same action. Additionally, there is 
no memory alteration conflict, as it only performs simultaneous reads and not writes. If the requested segment is different from 
what is already uploaded, the child proceeds to the else part of the if and submits a new request. Then it waits to be served.

**Log Files**
------------

Each child creates its own log file, named according to the child id (logfile_of_child_#XXXX.txt), to make it easier for the user 
to find it after executing the code. The log files of the children are kept in the working directory even if we want to run the 
program multiple times with different user inputs. New log files will appear alongside the old ones. At the beginning of the parent's 
main code, it checks if there is a previous log file from a previous parent, and if so, it deletes it. Then it creates its new log file, 
containing the project requirements (logfile_of_PARENT.txt).

**Makefile and Running the Code**
-------------------------------

Using the command "<make all>" compiles the source files. The command "<./OSHW_1>" executes the code.
