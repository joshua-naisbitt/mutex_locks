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
    int params[2];
};

void lock(int writefd, int readfd, struct SystemRequest request){
    printf("Requesting lock on Server Mutex...\n");
    request.call_num = 6;
    char name[50];
    snprintf(name, 50, "client%dfifo", request.pid);

    write(writefd, &request.call_num, sizeof(int));
    write(writefd, &name, 50);
    write(writefd, &request.pid, sizeof(int));
    read(readfd, &request.params[0], sizeof(int));
    if (request.params[0] == 0){
        printf("--------------------\n");
        printf("Successfully locked mutex\n");
        printf("--------------------\n");
    }
    else {
        printf("--------------------\n");
        printf("This client already owns locked mutex\n");
        printf("--------------------\n");
    }
}

void unlock(int writefd, int readfd, struct SystemRequest request){
    request.call_num = 7;
    char name[50];
    snprintf(name, 50, "client%dfifo", request.pid);

    //communicate with server
    printf("Requesting unlock on Server Mutex...\n");
    write(writefd, &request.call_num, sizeof(int));
    write(writefd, &name, 50);
    write(writefd, &request.pid, sizeof(int));
    read(readfd, &request.params[0], sizeof(int));
    if (request.params[0] == 0){
        printf("--------------------\n");
        printf("Successfully unlocked mutex\n");
        printf("--------------------\n");
    }
    else {
        printf("--------------------\n");
        printf("Request denied, you do not have access to the mutex.\n");
        printf("Mutex is currently held by Client %d.\n", request.params[0]);
        printf("--------------------\n");
    }
}

int main(){
    printf("Lauching program2 mainclient.c...\n");

    //generate processID
    struct SystemRequest request;
    request.pid = getpid();

    printf("PID: %d\n", request.pid);

    printf("opening server fifo...\n");
    //open write fifo with error handling message
    int writefd = open("fifo1", O_WRONLY);
    if (writefd == -1){
        printf("Unexpected error opening server FIFO.\n");
        return 1;
    }

    //Fifo will block here until server program fifos sync
    printf("opened FIFOs\n");

    //Create client fifo
    //TODO: Set name to include unique Identifier
    char name[50];
    snprintf(name, 50, "client%dfifo", request.pid);
    if ((mkfifo(name, 0777) < 0) && (errno != EEXIST)){
        printf("Unable to create fifo %s\n", name);
    }
    printf("Created client fifo %s\n", name);

    //Write client fifo to server
    request.call_num = 0;
    write(writefd, &request.call_num, sizeof(int));
    write(writefd, &name, 50);
    write(writefd, &request.pid, sizeof(int));
    printf("Wrote string: %s\n", name);

    //Open client fifo
    printf("opening %s\n", name);
    //open read fifo with error handling message
    int readfd = open(name, O_RDONLY);
    // THIS IS WHERE IT'S HANGING
    if (readfd == -1){
        printf("Unexpected error opening read only FIFO.\n");
        return 2;
    }
    printf("opened %s\n", name);

    while (1)
    {
        int choice;

        printf("Make a selection\n");
        printf("1 - Send Request to Server\n");
        printf("2 - EXIT client\n");
        printf("3 - TERMINATE server \n");
        scanf("%d", &choice);
        
        switch (choice)
        {
        case 1: //Send request
            printf("Choose a System Call\n");
            printf("2 - Double Number\n");
            printf("3 - Triple Number\n");
            printf("4 - Store\n");
            printf("5 - Recall\n");
            printf("6 - Lock Mutex\n");
            printf("7 - Unlock Mutex\n");
            printf("8 - Log clients to server\n");
            printf("9 - Process Critical Section\n");
            printf("Input selection: \n");
            scanf("%d", &request.call_num);
            printf("Sending input\n");
            

            switch (request.call_num)
            {
            case 2: //Double - 1 param
                printf("--------------------\n");
                printf("Double selected with 1 parameter\n");
                //input parameter
                printf("Input parameter 1: ");
                scanf("%d", &request.params[0]);
                //communicate with server
                printf("Requesting double with parameter '%d'...\n"), request.params[0];
                write(writefd, &request.call_num, sizeof(int));
                write(writefd, &name, 50);
                write(writefd, &request.pid, sizeof(int));
                write(writefd, &request.params[0], sizeof(int));
                printf("Wrote to server, reading from %s\n", name);
                read(readfd, &request.params[0],sizeof(int));
                printf("--------------------\n");
                printf("Calculated Value: %d\n", request.params[0]);
                printf("--------------------\n");
                break;
            case 3: //Triple - 1 param
                printf("Triple selected with 1 parameter\n");
                //input parameter
                printf("Input parameter 1: ");
                scanf("%d", &request.params[0]);
                //communicate with server
                printf("Requesting triple with parameter '%d'...\n"), request.params[0];
                write(writefd, &request.call_num, sizeof(int));
                write(writefd, &name, 50);
                write(writefd, &request.pid, sizeof(int));
                write(writefd, &request.params[0], sizeof(int));
                read(readfd, &request.params[0],sizeof(int));
                printf("--------------------\n");
                printf("Calculated Value: %d\n", request.params[0]);
                printf("--------------------\n");
                break;
            case 4: //Store - 2 params (params[0] = value, params[1] = memory)
                
                printf("Store selected with 2 parameters\n");
                //input parameters
                printf("Input parameter 1 (Value): ");
                scanf("%d", &request.params[0]);
                printf("Input parameter 2 (index in memory array to store value): ");
                scanf("%d", &request.params[1]);
                //communicate with server
                printf("--------------------\n");
                printf("Storing '%d' in memory[%d]...\n", request.params[0],request.params[1]);
                printf("--------------------\n");
                write(writefd, &request.call_num, sizeof(int));
                write(writefd, &name, 50);
                write(writefd, &request.pid, sizeof(int));
                //Send params to server
                write(writefd, &request.params[0], sizeof(int));
                write(writefd, &request.params[1], sizeof(int));
                break;
            case 5: //Recall - 1 param
                

                printf("Recall selected with 1 parameter\n");
                printf("Input parameter (index of memory[] to retrieve from): ");
                scanf("%d", &request.params[0]);
                //communicate with server
                printf("Retrieving value from memory[%d]...\n", request.params[0]);
                write(writefd, &request.call_num, sizeof(int));
                write(writefd, &name, 50);
                write(writefd, &request.pid, sizeof(int));
                write(writefd, &request.params[0], sizeof(int));
                read(readfd, &request.params[1], sizeof(int));
                printf("--------------------\n");
                printf("Retrieved value: %d\n", request.params[1]);
                printf("--------------------\n");
                break;

            case 6: //Lock
                printf("Lock selected\n");
                //communicate with server
                lock(writefd, readfd, request);
                break;

            case 7: //unlock - 1 param

                printf("Unlock selected\n");
                //communicate with server
                unlock(writefd, readfd, request);
                break;

            case 8: //ps

                printf("ps selected\n");
                //communicate with server
                
                write(writefd, &request.call_num, sizeof(int));
                write(writefd, &name, 50);
                write(writefd, &request.pid, sizeof(int));

                printf("--------------------\n");
                printf("client information logged to server console\n");
                printf("--------------------\n");
                break;
            
            case 9: //run critical section
                printf("Execute critical section selected\n");
                printf("Running non-critical section\n");
                for (int i = 0; i < 10; i++){
                    sleep(1);
                    printf(".\n");
                }
                printf("--------------------\n");
                printf("Non-critical section complete\n");
                printf("--------------------\n");
                lock(writefd, readfd, request);
                printf("Running critical section\n");
                for (int i = 0; i < 10; i++){
                    sleep(1);
                    printf(".\n");
                }
                printf("--------------------\n");
                printf("Critical section complete\n");
                printf("--------------------\n");
                unlock(writefd, readfd, request);
                break;


            default:
                break;
            }
            break;
        case 2: //EXIT
            printf("exiting...\n");
            request.call_num = -1;
            request.params[0] = 0;
            
            
            write(writefd, &request.call_num, sizeof(int));
            write(writefd, &name, 50);
            write(writefd, &request.pid, sizeof(int));
            write(writefd, &request.params[0], sizeof(int));
            unlink(name);
            break;
        case 3: //TERMINIATE
            printf("Termimating...\n");
            request.call_num = -1;
            request.params[0] = 1;
            
            write(writefd, &request.call_num, sizeof(int));
            write(writefd, &name, 50);
            write(writefd, &request.pid, sizeof(int));
            write(writefd, &request.params[0], sizeof(int));
            unlink(name);
            break;
        }
        
        if (request.call_num == -1){
            break;
        }
    }

    //close fifos
    close(writefd);
    close(readfd);
    unlink(name);
    printf("closed fifos\n");

    return 0;
}
