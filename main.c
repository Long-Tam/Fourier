#include "routine.h"

pthread_t t2, t3;

sem_t flag1;
sem_t flag2;

int main()
{
    sem_init(&flag1, 0, 0);
    sem_init(&flag2, 0, 0);

    // pthread_create(&t1, NULL, routine1, NULL);
    pthread_create(&t2, NULL, routine2, NULL);
    pthread_create(&t3, NULL, routine3, NULL);
    routine1();

    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
}
