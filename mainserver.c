#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

struct SystemRequest{
    int pid;
    int call_num;
    int num_params;
    int param_size;
    int params[2];
};

int mutex = 0; // value of mutex = process currently holding it, 0 = unlocked, 
int wait_queue[100]; //queue of clients waiting for mutex max clients = 100
int num_waiting = 0;
int num_clients = 0;
int connected_clients[100]; //list of all connected clients

int main(){
    printf("Lauching program2 mainserver.c...\n");
    if (mkfifo("fifo1", 0777) == -1){
        if (errno != EEXIST) {
            printf("Could not create fifo file\n");
            return 1;
        }
    };
    struct SystemRequest request;
    int memory[256];
    //open server fifo as read only with error handling message
    printf("opening server fifo\n");
    int readfd = open("fifo1", O_RDONLY);
    if (readfd == -1){
        printf("Unexpected error opening read only FIFO.\n");
        return 1;
    }

    //FIFOs will block here until client program opens FIFOs
    printf("opened FIFO\n");
    
    //read name and id of client fifo
    char name[50];
    int pid;
    read(readfd, &request.call_num, sizeof(int));
    read(readfd, &name, 50);
    read(readfd, &pid, sizeof(int));
    printf("Client pid: %d\n", pid);
    printf("System Call 0 Requested with 1 parameter: %s\n", name);


    //open client fifo as write only with error handling message
    printf("opening %s\n", name);
    int writefd = open(name, O_WRONLY);
    if (writefd == -1){
        printf("Unexpected error opening write only FIFO.\n");
        return 2;
    }
    printf("opened %s\n", name);
    printf("--------------------\n");
    connected_clients[num_clients++] = pid;


    while (1){
        printf("Waiting to read system call\n");
        read(readfd, &request.call_num, sizeof(int));
        printf("Accepted system call: %d\n", request.call_num);

        switch (request.call_num)
        {
        case 0: //Connect Client
            printf("System Call 0 Requested\n");
            // read(readfd, &request.call_num, sizeof(int));
            read(readfd, &name, 50);
            printf("System Call 0 Requested with 1 parameter: %s\n", name);
            read(readfd, &pid, sizeof(int));
            printf("recieved PID: %d\n", pid);
            
            //open client fifo as write only with error handling message
            printf("opening %s\n", name);
            int writefd = open(name, O_WRONLY);
            if (writefd == -1){
                printf("Unexpected error opening write only FIFO.\n");
                return 2;
            }
            printf("opened %s\n", name);
            printf("--------------------\n");
            connected_clients[num_clients++] = pid;
            break;
        case 2: //Double Number
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            read(readfd, &request.params[0],sizeof(int));
            printf("Client pid: %d\n", pid);
            printf("System Call 2 Requested with 1 parameter: %d\n", request.params[0]);
            request.params[0]*=2;
            printf("Calculated Value: %d\n", request.params[0]);
            writefd = open(name, O_WRONLY);
            write(writefd, &request.params[0],sizeof(int));
            printf("Wrote parameter %d to: %s\n", request.params[0], name);
            printf("--------------------\n");
            break;
        case 3: //Triple Number
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            read(readfd, &request.params[0],sizeof(int));
            printf("Client pid: %d\n", pid);
            printf("System Call 3 Requested with 1 parameter: %d\n", request.params[0]);
            request.params[0]*=3;
            printf("Calculated Value: %d\n", request.params[0]);
            writefd = open(name, O_WRONLY);
            write(writefd, &request.params[0],sizeof(int));
            printf("Wrote parameter %d to: %s\n", request.params[0], name);
            printf("--------------------\n");
            break;
        case 4: //Store
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            read(readfd, &request.params[0],sizeof(int));
            read(readfd, &request.params[1],sizeof(int));
            printf("Client pid: %d\n", pid);
            printf("System Call 4 Requested with 2 parameters: %d, %d\n", request.params[0],request.params[1]);
            memory[request.params[1]]=request.params[0];
            printf("Stored '%d' in memory[%d]\n", request.params[0],request.params[1]);
            printf("--------------------\n");
            break;
        case 5: //Recall
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            read(readfd, &request.params[0],sizeof(int));
            printf("Client pid: %d\n", pid);
            printf("System Call 5 Requested with 1 parameter: %d\n", request.params[0]);
            writefd = open(name, O_WRONLY);
            write(writefd, &memory[request.params[0]], sizeof(int));
            printf("--------------------\n");
            break;


        ///    
        case 6: //Lock Mutex
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            printf("Client pid: %d\n", pid);
            printf("System Call 6 Requested\n");
            printf("Processing request to lock server mutex...\n");
            //read mutex int
            //if mutex unlocked, lock and reply 
            //else, put client into wait queue until mutux is unlocked, then lock and reply
            if (mutex == 0){
                mutex = pid;
                printf("Mutex locked by Client %d Sucessfully\n", pid);
                writefd = open(name, O_WRONLY);
                printf("writefd opened %s\n", name);
                request.params[0] = 0; //mutex sucessfully locked 
                write(writefd, &request.params[0],sizeof(int));
                printf("write succeeded\n");
            }
            else if (mutex == pid){
                printf("--------------------\n");
                printf("Mutex already locked by requesting client %d \n", pid);
                printf("--------------------\n");
                writefd = open(name, O_WRONLY);
                request.params[0] = 1; //mutex already locked 
                write(writefd, &request.params[0],sizeof(int));
            }
            else{
                printf("--------------------\n");
                printf("Mutex already locked by Client %d, adding Client %d to wait queue\n", mutex, pid);
                printf("--------------------\n");
                wait_queue[num_waiting++] = pid;
            }
            break;

        case 7: //unlock Mutex
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            printf("Client pid: %d\n", pid);
            printf("System Call 7 Requested\n");
            printf("Processing request to lock server mutex...\n");
            //if mutex unlocked, or client holds mutex, unlock
            if (mutex == 0 || mutex == pid){
                mutex = 0;
                printf("--------------------\n");
                printf("Mutex unlocked sucessfully\n");
                writefd = open(name, O_WRONLY);
                request.params[0] = 0; //mutex unlock granted 
                write(writefd, &request.params[0],sizeof(int));
                
                //process waiting queue
                if (num_waiting > 0){
                    mutex = wait_queue[0];
                    for (int i = 0; i < num_waiting - 1; i++){
                        wait_queue[i] = wait_queue[i + 1];
                    }
                    num_waiting--;
                    //communicate mutex control to appropriate fifo
                    snprintf(name, 50, "client%dfifo", mutex);
                    writefd = open(name, O_WRONLY);
                    request.params[0] = 0; //mutex sucessfully locked 
                    write(writefd, &request.params[0],sizeof(int));
                    printf("Mutex assigned to Client %d\n", mutex);
                    

                }
                printf("--------------------\n");
            }
            //Mutex locked by another client
            else{
                printf("Cannot unlock Mutex, Mutex is currently held by Client %d\n", mutex);
                writefd = open(name, O_WRONLY);
                request.params[0] = mutex; //mutex unlock denied 
                write(writefd, &request.params[0],sizeof(int));
            }
            break;

        case 8: //ps
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            printf("Client pid: %d\n", pid);
            printf("System Call 8 Requested\n");
            printf("Current number of clients: %d\n", num_clients);
            printf("Current number of waiting clients: %d\n", num_waiting);
            printf("--------------------\n");
            printf("Connected client PIDs:\n");
            for (int i = 0; i < num_clients; i++){
                    printf("%d",connected_clients[i]);
                    if (mutex == connected_clients[i]){
                        printf(" MUTEX OWNER");
                    }
                    else{
                        for (int j = 0; j < num_waiting; j++){
                            if (connected_clients[i] == wait_queue[j]){
                                printf(" WAITING");
                            }
                        }
                    }
                    printf("\n");
                }
            printf("--------------------\n");
            break;

        default:
            break;
        }
        if (request.call_num == -1){
            read(readfd, &name, 50);
            read(readfd, &pid, sizeof(int));
            read(readfd, &request.params[0],sizeof(int));
            printf("client %d exited\n", pid);
            if (num_clients > 0){
                    int tmp = 0;
                    for (int i = 0; i < num_clients; i++){
                        if (connected_clients[i] == pid){
                            tmp = i;
                            printf("client %d found at index %d\n",pid, tmp);
                            break;
                        }
                    }

                    for (int i = tmp; i < num_clients- 1; i++){
                        connected_clients[i] = connected_clients[i + 1];
                    }
                    num_clients--;

                }
            close(writefd);
            close(readfd);

            if (request.params[0] == 1){
                break; //terminate condition
            }
            
            //Waiting for new client
            readfd = open("fifo1", O_RDONLY);
            if (readfd == -1){
                printf("Unexpected error opening read only FIFO.\n");
                return 1;
            }
        }
    }
    
    //close FIFOs
    close(readfd);
    unlink("fifo1");
    printf("closed fifos\n");

    return 0;
}
