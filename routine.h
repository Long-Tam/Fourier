#include <pthread.h>
#include <semaphore.h>
#include "func.h"


extern sem_t flag1;
extern sem_t flag2;

void *routine1();
void *routine2();
void *routine3();