#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main ()
{
    char data = ' ';
    char * myfifo = "/tmp/myfifo";
    
    int fd = open(myfifo, O_RDWR);
    
    if (fd == 0) 
    {
        perror("Cannot open fifo");
        unlink(myfifo);exit(1);
        
    }
    printf("Waiting for characters.....\n");
    while (data != '#') 
    {
        while (read(fd, &data, 1) && (data != '#'))
            fprintf(stderr, "%c", data);
        
    }
    close(fd);
    unlink(myfifo);
    return 0;
    
}