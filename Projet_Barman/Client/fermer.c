#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Client.h"
#include "Tireuse.h"


int main(int argc, char** argv)
{
    //struct tireuse* tireuse;
   key_t key = ftok("Tireuse", 'A');
    int shmid = shmget(key, 2*sizeof(struct tireuse), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    /*
    printf("status = %d\n", status);
    if ((tireuse = (struct tireuse*)shmat(status, NULL, 0)) == (struct tireuse*)-1)
    {
        printf("shm2.shmat: %s\n", strerror(errno));
        exit(1);
    }
    
        if (shmdt(tireuse) == -1)
        {
        printf("shm2.shmdt: %s\n", strerror(errno));
        exit(1);
        } */
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
        {
        printf("shm2.shmctl: %s\n", strerror(errno));
        exit(1);
        }
    

}
