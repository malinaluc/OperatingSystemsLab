#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int fdreq = -1;
int fdresp = -1;

// comunica pipe ului mesajul in formatul cerut
void writeStringField(char* string)
{
    char aux[strlen(string) + 2];
    strcpy(aux, string);
    aux[strlen(string)] = '!';
    aux[strlen(string) + 1] = '\0';

    write(fdresp, aux, strlen(aux));
}

void writeNumberField(unsigned number)
{
    // scriere directa numar de tip unsigned pe 4 bytes
    write(fdresp, &number, 4);
}


void readStringField(char* buffer)
{
    char util;
    int i = 0;
    while (1)
    {
        if (read(fdreq, &util, 1) == -1)
            return;
        if (util == '!')
            break;
        buffer[i++] = util;
    }
    buffer[i] = '\0';
}

unsigned readNumberField()
{
    unsigned number;
    if (read(fdreq, &number, 4) == -1)
        return -1;
    return number;
}

int main()
{

    char *reqPipe = "REQ_PIPE_58203";
    char *respPipe = "RESP_PIPE_58203";

    ///unlink(respPipe); ///la rulari consecutive, as primi eroare deoarece ar fi deja creat

    /// 0600 - > permisiune atat pentru scriere, cat si pentru citire <le am lasat doar pentru user>
    if(mkfifo(respPipe,0600)!= 0)
    {
        perror("ERROR\n"
               "cannot create the response pipe\n");
        return 1;
    }

    fdreq = open(reqPipe,O_RDONLY);
    if(fdreq == -1)
    {
        perror("ERROR\n"
               "cannot open the request pipe");
        return 1;

    }

    fdresp = open(respPipe,O_WRONLY);
    if(fdresp== -1)
    {
        perror("Nu s a putut deschide in scriere “RESP_PIPE_58203”");
        return 1;

    }

    // comunica piep ului mesajul de start pentru a creea conexiunea
    writeStringField("HELLO");

    char request[100];
    while(1)
    {
        // citeste continuu stringuri din pipe dupa le compara
        readStringField(request);

        // ex2.3
        if (strcmp(request, "VARIANT") == 0)
        {
            writeStringField("VARIANT");
            writeStringField("VALUE");
            writeNumberField(58203);
        }
        //ex2.4
        else if (strcmp(request, "CREATE_SHM") == 0)
        {
            unsigned number = readNumberField();
            int shmid;
            void *shmaddr;

            key_t key = ftok(".", 'R');
            if (key == -1)
            {
                perror("Eroare la generarea cheii IPC");
                exit(1);
            }

            shmid = shmget(key, number, IPC_CREAT | 664);
            if (shmid == -1)
            {
                perror("Eroare la crearea regiunii de memorie partajată");
                exit(1);
            }

            off_t size = number;
            if (ftruncate(shmid, size) == -1)
            {
                perror("ftruncate");
                return 1;
            }

            shmaddr = shmat(shmid, NULL, 0);
            if (shmaddr == (void*) -1)
            {
                perror("Eroare la atașarea regiunii de memorie partajată");
                exit(1);
            }

            if (shmdt(shmaddr) == -1)
            {
                perror("Eroare la eliberarea regiunii de memorie partajată");
                exit(1);
            }



        }
        //ex2.5
        else if (strcmp(request, "WRITE_TO_SHM") == 0)
        {

        }
        //ex2.6
        else if (strcmp(request, "MAP_FILE") == 0)
        {

        }
        //ex2.7
        else if (strcmp(request, "READ_FROM_FILE_OFFSET") == 0)
        {

        }
        //ex2.8
        else if (strcmp(request, "READ_FROM_FILE_SECTION") == 0)
        {

        }
        //ex2.9
        else if (strcmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {

        }
        // ex2.10 comanda de iesire
        else if (strcmp(request, "EXIT") == 0)
        {
            break;
        }
    }


    close(fdreq);
    close(fdresp);
    unlink(reqPipe);
    unlink(respPipe);

    return 0;
}

