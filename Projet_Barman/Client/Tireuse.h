#ifndef TIREUSE_H
#define TIREUSE_H

struct tireuse
{
    char type_biere[20];
    float quantite_biere;
    char nom_biere[20];
};

int create_semaphore(key_t key);
void lock_semaphore(int semid);
void unlock_semaphore(int semid);


void creer_shm();

#endif