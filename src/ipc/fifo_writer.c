#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main ()
{
    char data = ' ';
    size_t len = 0;     /* ignored when line = NULL */
    ssize_t read;
    char * myfifo = "/tmp/myfifo";
    if (mkfifo(myfifo, S_IRUSR | S_IWUSR) != 0)
        perror("Cannot create fifo. Already existing?");
    
    int fd = open(myfifo, O_RDWR);
    if (fd == 0) 
    {
        perror("Cannot open fifo");
        unlink(myfifo);
        exit(1);
        
    }

    printf ("Chatting below, digit # to quit\n");
    while (data != '#') 
    {
    scanf("%c",&data);
    int nb = write(fd, &data,1);
        if (nb == 0)
        {
            fprintf(stderr,"Write error\n");
            exit(1);
        }
    }
    //close(fd);
    //unlink(myfifo);
   
    return 0;
    
}