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
void *shmaddr;
void *file_ptr;

off_t shared_mem_size ;

struct stat file_stat;

int fd; ///fisierul din memoria partajata

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

    writeStringField("HELLO");

    char request[100];

    while(1)
    {
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
            shmid = shm_open("/q7jfuq", O_CREAT| O_RDWR, 0664);
            if (shmid == -1)
            {
                writeStringField("CREATE_SHM!ERROR!");
                exit(1);
            }

            shared_mem_size = number;
            if (ftruncate(shmid, shared_mem_size) == -1)
            {
                writeStringField("CREATE_SHM!ERROR!");
                return 1;
            }

            shmaddr = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED,shmid, 0);
            if (shmaddr == (void*) -1)
            {
                writeStringField("CREATE_SHM!ERROR!");
                exit(1);
            }

            writeStringField("CREATE_SHM!SUCCESS!");

        }
        //ex2.5
        else if (strcmp(request, "WRITE_TO_SHM") == 0)
        {
            unsigned offset = readNumberField();
            unsigned value = readNumberField(); ///valoarea de scris in memorie la offset

            if (offset <0 || offset>=3613231 || offset >= shared_mem_size + sizeof(value))
            {
                writeStringField("WRITE_TO_SHM!ERROR!");
                return 1;
            }

            unsigned *ptr = (unsigned *)((char *)shmaddr + offset);
            *ptr = value;
            memcpy((void *)((char *)shmaddr + offset), &value, sizeof(unsigned));

            writeStringField("WRITE_TO_SHM!SUCCESS!");

        }
        //ex2.6
        else if (strcmp(request, "MAP_FILE") == 0)
        {

            char nume_fisier[100];
            readStringField(nume_fisier);

            fd = open(nume_fisier,O_RDONLY);
            if(fd==-1)
            {
                writeStringField("MAP_FILE!ERROR!");
                return 1;
            }

            if (fstat(fd, &file_stat) == -1)
            {
                writeStringField("MAP_FILE!ERROR!");
                return 1;
            }


            file_ptr = mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
            if (file_ptr == MAP_FAILED)
            {
                writeStringField("MAP_FILE!ERROR!");
                munmap(shmaddr, file_stat.st_size);
                close(fd);
                close(shmid);
                return 1;
            }

            writeStringField("WRITE_TO_SHM!SUCCESS!");

        }
        //ex2.7
        else if (strcmp(request, "READ_FROM_FILE_OFFSET") == 0)
        {
            unsigned offset = readNumberField();
            unsigned no_of_bytes = readNumberField();

            if(offset < 0 || offset+no_of_bytes>= file_stat.st_size)
            {
                writeStringField("READ_FROM_FILE_OFFSET!ERROR!");
                return 1;
            }

            if(fstat(fd,&file_stat) ==-1)
            {
                writeStringField("READ_FROM_FILE_OFFSET!ERROR!");
                exit(EXIT_FAILURE);
            }

            off_t file_size = file_stat.st_size;

            void *file_mapping = mmap(NULL, file_size, PROT_READ,MAP_SHARED,fd,0);

            if (file_mapping == (void*) -1)
            {
                writeStringField("READ_FROM_FILE_OFFSET!ERROR!");
                exit(1);
            }

            char *file_data = (char*)file_mapping;
            for(off_t i = offset ; i<no_of_bytes; i++)
            {
                char curr_oct = file_data[i];
                printf("%c", curr_oct);
            }

            munmap(file_mapping, file_size);
            close(fd);

            writeStringField("WRITE_TO_SHM!SUCCESS!");
        }
        //ex2.8
        else if (strcmp(request, "READ_FROM_FILE_SECTION") == 0)
        {

            writeStringField("WRITE_TO_SHM!SUCCESS!");
        }
        //ex2.9
        else if (strcmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {

            writeStringField("WRITE_TO_SHM!SUCCESS!");
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

