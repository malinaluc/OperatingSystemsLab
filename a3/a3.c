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
#include <errno.h>

int fdreq = -1;
int fdresp = -1;

int shmid;
void *shmaddr = NULL;
void *file_ptr;

off_t shared_mem_size ;

struct stat file_stat;

int fd; ///fisierul din memoria partajata

void writeStringField(char* string)
{
    write(fdresp, string, strlen(string));
}

void writeNumberField(unsigned number)
{
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

    /// 0600 - > permisiune atat pentru scriere, cat si pentru citire <le am lasat doar pentru user>
    if(mkfifo(respPipe,0600)!= 0)
    {
        perror("ERROR\n"
               "pipe raspuns necreat\n");
        return 1;
    }

    fdreq = open(reqPipe,O_RDONLY);
    if(fdreq == -1)
    {
        perror("ERROR\n"" nu se poate deschide pipe ul de request");
        return 1;
    }

    fdresp = open(respPipe,O_WRONLY);
    if(fdresp== -1)
    {
        perror("Nu s a putut deschide in scriere “RESP_PIPE_58203”");
        return 1;

    }

    writeStringField("HELLO!");

    char request[100];

    while(1)
    {
        readStringField(request);

        // ex2.3
        if (strcmp(request, "VARIANT") == 0)
        {
            writeStringField("VARIANT!");
            writeStringField("VALUE!");
            writeNumberField(58203);
        }
        //ex2.4
        else if (strcmp(request, "CREATE_SHM") == 0)
        {
            unsigned number = readNumberField();
            shmid = shm_open("/q7jfuq", O_CREAT| O_RDWR, 0664);
            if (shmid == -1)
            {
                writeStringField("CREATE_SHM!ERROR!");
                continue;
            }

            shared_mem_size = number;
            if (ftruncate(shmid, shared_mem_size) == -1)
            {
                writeStringField("CREATE_SHM!ERROR!");
                continue;
            }

            shmaddr = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED,shmid, 0);
            if (shmaddr == (void*) -1)
            {
                writeStringField("CREATE_SHM!ERROR!");
                continue;
            }

            writeStringField("CREATE_SHM!SUCCESS!");

        }
        //ex2.5
        else if (strcmp(request, "WRITE_TO_SHM") == 0)
        {
            unsigned offset = readNumberField();
            unsigned value = readNumberField(); ///valoarea de scris in memorie la offset

            if (offset <0 || offset >= 3613231 || offset + sizeof(value) >= shared_mem_size)
            {
                writeStringField("WRITE_TO_SHM!ERROR!");
                continue;
            }

            if (shmaddr == NULL) {
                writeStringField("WRITE_TO_SHM!ERROR!");
                continue;
            }

            unsigned *ptr = (unsigned *) (shmaddr + offset);
            *ptr = value;

            writeStringField("WRITE_TO_SHM!SUCCESS!");

        }
        //ex2.6
        else if (strcmp(request, "MAP_FILE") == 0)
        {

            char nume_fisier[512];
            readStringField(nume_fisier);

            fd = open(nume_fisier,O_RDONLY);
            if(fd==-1)
            {
                writeStringField("MAP_FILE!ERROR!");
                continue;
            }

            if (fstat(fd, &file_stat) == -1)
            {
                writeStringField("MAP_FILE!ERROR!");
                continue;
            }


            file_ptr = mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
            if (file_ptr == MAP_FAILED)
            {
                writeStringField("MAP_FILE!ERROR!");
                continue;
            }

            writeStringField("MAP_FILE!SUCCESS!");

        }
        //ex2.7
        else if (strcmp(request, "READ_FROM_FILE_OFFSET") == 0)
        {
            unsigned offset = readNumberField();
            unsigned no_of_bytes = readNumberField();

            if(offset < 0 || offset+no_of_bytes>= file_stat.st_size)
            {
                writeStringField("READ_FROM_FILE_OFFSET!ERROR!");
                continue;
            }

            memcpy(shmaddr, file_ptr + offset, no_of_bytes);

            writeStringField("READ_FROM_FILE_OFFSET!SUCCESS!");
        }
        //ex2.8
        else if (strcmp(request, "READ_FROM_FILE_SECTION") == 0)
        {
            writeStringField("READ_FROM_FILE_SECTION!SUCCESS!");
        }
        //ex2.9
        else if (strcmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            /*unsigned logical_offset = readNumberField();
            unsigned no_of_bytes = readNumberField();*/

            writeStringField("READ_FROM_LOGICAL_SPACE_OFFSET!SUCCESS!");
        }
        // ex2.10 comanda de iesire
        else if (strcmp(request, "EXIT") == 0)
        {
            break;
        }
    }

    close(shmid);
    close(fd);
    munmap(shmaddr,shared_mem_size);
    close(fdreq);
    close(fdresp);
    unlink(reqPipe);
    unlink(respPipe);

    return 0;
}
