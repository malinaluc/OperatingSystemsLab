#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#define NUMBER_OF_THREADS_FOR_P2 5



typedef struct thread_data
{
    int process_number;
    int thread_number;
} thread_data;

int current_thread_sincronizare = 1;

void thread_function_sincronizare(void *arg) ///2.3
{
    struct thread_data *data = (struct thread_data *)arg;
    int process_num = data->process_number;
    int thread_num = data->thread_number;

    while(thread_num > current_thread_sincronizare) ///asteptam sa fie randul thread ului curent sa inceapa, pentru a ne asigura ca thread ul 1 va incepe inaintea thread-ului 3
    {
        intptr_t waiting = (intptr_t)pthread_self();
        if(waiting) continue;
    }
    info(BEGIN,process_num,thread_num);
    current_thread_sincronizare++;

    if(thread_num != 0 && thread_num!=1)
    {
        info(END,process_num,thread_num);

    }
    pthread_exit(NULL);

}

sem_t sem;
int wait_barrier = 1;

void thread_function_bariera_v1(void *arg) ///2.4
{
    struct thread_data *data = (struct thread_data *)arg;
    int process_num = data->process_number;
    int thread_num = data->thread_number;

    info(BEGIN,process_num,thread_num);

    if (thread_num != 10) {
        sem_wait(&sem);
        while (wait_barrier);
    }

    info(END,process_num,thread_num);

    if (thread_num == 10) {
        sem_post(&sem);
        wait_barrier = 0;
        sem_destroy(&sem);
    }

    pthread_exit(NULL);
}

void thread_function_normal(void *arg) ///2.4
{
    struct thread_data *data = (struct thread_data *)arg;
    int process_num = data->process_number;
    int thread_num = data->thread_number;

    info(BEGIN,process_num,thread_num);
    info(END,process_num,thread_num);

    pthread_exit(NULL);
}

sem_t *sem1, *sem2;

int main()
{
    init();

    info(BEGIN, 1, 0);

    sem_unlink("/sema1");
    sem_unlink("/sema2");

    sem1 = sem_open("/sema1", O_CREAT, 0600, 0);
    sem2 = sem_open("/sema2", O_CREAT, 0600, 0);

    pid_t P2=-1, P3=-1, P4=-1, P5=-1, P6=-1, P7=-1, P8=-1, P9=-1;

    ///cream procesul 2
    P2 = fork();
    if(P2 < 0)
    {
        puts("Eroare la creare P2");

    }
    else if(P2 == 0)
    {
        ///procesul 2 incepe
        info(BEGIN,2,0);
        ///cream procesul 3
        P3 = fork();
        if(P3 < 0)
        {
            puts("Eroare la creare P3");

        }
        else if(P3 == 0)
        {
            ///P3 incepe
            info(BEGIN,3,0);

            ///crearea thread-urilor

            struct thread_data data[6];

            pthread_t threads[6];
            for (int i = 0; i < 6; ++i) {
                data[i].process_number = 3;
                data[i].thread_number=i+1;
            }

            pthread_create(&threads[0],NULL,(void *) thread_function_normal,(void *)&data[0]);
            pthread_join(threads[0],NULL);

            sem_post(sem1);
            sem_wait(sem2);

            for(int i=1; i<6; i++)
            {
                int tc = pthread_create(&threads[i],NULL,(void *) thread_function_normal,(void *)&data[i]);
                if(tc)
                {
                    printf("Eroare la crearea thread-ului %d\n",i+1);
                    exit(1);
                }
            }

            ///asteptam terminarea fiecarui thread
            for(int i=1; i<6; i++)
            {
                pthread_join(threads[i],NULL);
            }


            ///P3 se termina
            info(END,3,0);
            exit(0);

        }

        ///cream P8
        P8 = fork();
        if(P8 < 0)
        {
            puts("Eroare la creare P8");

        }
        else if(P8 == 0)
        {
            ///P8 incepe
            info(BEGIN,8,0);


            ///crearea thread-urilor
            pthread_t threads[37];
            struct thread_data data[37];
            for (int i = 0; i < 37; ++i) {
                data[i].process_number = 8;
                data[i].thread_number = i + 1;
            }

            sem_init(&sem, 1, 5);

            for (int i = 8; i < 12; i++) {

                int tc = pthread_create(&threads[i], NULL, (void *) thread_function_bariera_v1, (void *) &data[i]);
                if (tc) {
                    printf("Eroare la crearea thread-ului %d\n", i + 1);
                    exit(1);
                }

            }

            ///asteptam terminarea fiecarui thread
            for (int i = 8; i < 12; i++) {
                pthread_join(threads[i], NULL);
            }

            for(int i = 0; i < 36; i+= 4){
                if (i == 8){
                    continue;
                }
                int tc1 = pthread_create(&threads[i], NULL, (void *) thread_function_normal, (void *) &data[i]);
                int tc2 = pthread_create(&threads[i + 1], NULL, (void *) thread_function_normal, (void *) &data[i + 1]);
                int tc3 = pthread_create(&threads[i + 2], NULL, (void *) thread_function_normal, (void *) &data[i + 2]);
                int tc4 = pthread_create(&threads[i + 3], NULL, (void *) thread_function_normal, (void *) &data[i + 3]);
                if (tc1) {
                    printf("Eroare la crearea thread-ului %d\n", i + 1);
                    exit(1);
                }
                if (tc2) {
                    printf("Eroare la crearea thread-ului %d\n", i + 2);
                    exit(1);
                }
                if (tc3) {
                    printf("Eroare la crearea thread-ului %d\n", i + 3);
                    exit(1);
                }
                if (tc4) {
                    printf("Eroare la crearea thread-ului %d\n", i + 4);
                    exit(1);
                }
                pthread_join(threads[i], NULL);
                pthread_join(threads[i + 1], NULL);
                pthread_join(threads[i + 2], NULL);
                pthread_join(threads[i + 3], NULL);
            }

            int tc = pthread_create(&threads[36], NULL, (void *) thread_function_normal, (void *) &data[36]);
            if (tc) {
                printf("Eroare la crearea thread-ului %d\n", 37);
                exit(1);
            }
            pthread_join(threads[36], NULL);

            info(END,8,0);
            exit(0);

        }
        waitpid(P8,NULL,0);

        ///crearea thread-urilor
        pthread_t threads[5];

        sem_wait(sem1);

        for(int i=0; i<5; i++)
        {

            struct thread_data *data = malloc(sizeof(struct thread_data));
            data->process_number = 2;
            data->thread_number=i+1;
            int tc = pthread_create(&threads[i],NULL,(void *)thread_function_sincronizare,(void *)data);
            if(tc)
            {
                printf("Eroare la crearea thread-ului %d\n",i+1);
                exit(1);
            }

        }

        ///asteptam terminarea fiecarui thread
        for(int i=0; i<5; i++)
        {
            pthread_join(threads[i],NULL);
        }

        sem_post(sem2);

        info(END,2,1);

        waitpid(P3,NULL,0);

        ///P2 se termina
        info(END,2,0);

        exit(0);
    }

    waitpid(P2,NULL,0);

    ///cream P4
    P4 = fork();
    if(P4 < 0)
    {
        puts("Eroare la creare P4");

    }
    if(P4 == 0)
    {
        ///P4 incepe
        info(BEGIN,4,0);
        ///P4 se termina
        info(END,4,0);
        exit(0);
    }
    waitpid(P4,NULL,0);

    ///cream P5
    P5 = fork();
    if(P5 < 0)
    {
        puts("Eroare la creare P5");

    }
    if(P5 == 0)
    {
        ///P5 incepe
        info(BEGIN,5,0);
        ///P5 se termina
        info(END,5,0);
        exit(0);

    }
    waitpid(P5,NULL,0);

    ///cream P6
    P6 = fork();
    if(P6 < 0)
    {
        puts("Eroare la creare P6");
    }
    else if(P6==0)
    {
        ///P6 incepe
        info(BEGIN,6,0);

        ///cream P7
        P7 = fork();
        if(P7 < 0)
        {
            perror("Eroare la creare P7");

        }
        else if(P7 == 0)
        {
            ///P7 incepe
            info(BEGIN,7,0);
            ///P7 se termina
            info(END,7,0);
            exit(0);

        }
        waitpid(P7,NULL,0);

        ///cream P9
        P9 = fork();
        if(P9 < 0)
        {
            puts("Eroare la creare P9");
        }
        else if(P9 == 0)
        {
            ///P9 incepe
            info(BEGIN,9,0);
            ///P9 se termina
            info(END,9,0);
            exit(0);

        }
        waitpid(P9,NULL,0);

        ///P6 se termina
        info(END,6,0);
        exit(0);
    }
    waitpid(P6,NULL,0);


    ///P1 se termina
    info(END,1,0);

    return 0;
}
