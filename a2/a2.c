#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
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
        int waiting = (int)pthread_self();
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

int current_thread_bariera =1;

void thread_function_bariera(void *arg) ///2.4
{
    struct thread_data *data = (struct thread_data *)arg;
    int process_num = data->process_number;
    int thread_num = data->thread_number;

    while(thread_num != current_thread_bariera) ///asteptam sa fie randul thread ului curent sa inceapa, pentru a ne asigura ca thread ul 1 va incepe inaintea thread-ului 3
    {
        int waiting = (int)pthread_self();
        if(waiting) continue;
    }
    info(BEGIN,process_num,thread_num);

    if(current_thread_bariera%4==0) ///inchidem tot din 4 in 4 thread-uri
    {
        printf("Am ajuns la un thread divizibil cu 4 : %d\n",thread_num);
        if(current_thread_bariera == 12) ///ne aflam in "colectia" unde se afla si thread ul 10
        {
            printf("Am intrat in if-ul cu 12 : %d\n",thread_num);
            info(END,process_num,10); ///ne asiguram ca thread-ul 10 se incheie atunci cand ruleaza(impreuna cu el) 4 thread-uri simultan
            info(END,process_num,current_thread_bariera-3);
            info(END,process_num,current_thread_bariera-1);
            info(END,process_num,current_thread_bariera);
        }
        else
        {
            printf("Am intrat in if-ul dinainte de 12 : %d\n",thread_num);
            info(END,process_num,current_thread_bariera-3);
            info(END,process_num,current_thread_bariera-2);
            info(END,process_num,current_thread_bariera-1);
            info(END,process_num,current_thread_bariera);

        }

    }
    else if(current_thread_bariera == 37) info(END,8,37);

    current_thread_bariera++;

    pthread_exit(NULL);
}

int current_thread_proceseDiferite =1;

void thread_function_proceseDiferite(void *arg) ///2.5
{
    struct thread_data *data = (struct thread_data *)arg;
    int process_num = data->process_number;
    int thread_num = data->thread_number;

    while(thread_num > current_thread_proceseDiferite) ///asteptam sa fie randul thread ului curent sa inceapa, pentru a ne asigura ca thread ul 1 va incepe inaintea thread-ului 3
    {
        int waiting = (int)pthread_self();
        if(waiting) continue;
    }
    info(BEGIN,process_num,thread_num);
    current_thread_proceseDiferite++;

    info(END,process_num,thread_num);

    pthread_exit(NULL);

}

int main()
{
    init();

    info(BEGIN, 1, 0);

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

            pthread_t threads[6];

            for(int i=0; i<6; i++)
            {

                struct thread_data *data = malloc(sizeof(struct thread_data));
                data->process_number = 3;
                data->thread_number=i+1;
                int tc = pthread_create(&threads[i],NULL,(void *)thread_function_proceseDiferite,(void *)data);
                if(tc)
                {
                    printf("Eroare la crearea thread-ului %d\n",i+1);
                    exit(1);
                }
            }

            ///asteptam terminarea fiecarui thread
            for(int i=0; i<6; i++)
            {
                pthread_join(threads[i],NULL);
            }


            ///P3 se termina
            info(END,3,0);
            exit(0);

        }
        waitpid(P3,NULL,0);

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

            for(int i=0; i<37; i++)
            {

                struct thread_data *data = malloc(sizeof(struct thread_data));
                data->process_number = 8;
                data->thread_number=i+1;
                int tc = pthread_create(&threads[i],NULL,(void *)thread_function_bariera,(void *)data);
                if(tc)
                {
                    printf("Eroare la crearea thread-ului %d\n",i+1);
                    exit(1);
                }

            }

            ///asteptam terminarea fiecarui thread
            for(int i=0; i<37; i++)
            {
                pthread_join(threads[i],NULL);
            }

            ///P8 se termina
            ///info(END,8,37);
            info(END,8,0);
            exit(0);

        }
        waitpid(P8,NULL,0);

        ///crearea thread-urilor
        pthread_t threads[5];

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

        ///P2 se termina
        info(END,2,1);
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
    exit(0);

    return 0;
}
