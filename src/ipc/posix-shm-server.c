#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>


int main (int argc, char *argv[]) 
{
    const char * shm_name  = "/AOS";
    const int SIZE = 4096;
    const char * message[] = {"This ","is ","about ","shared ","memory"};
    int i, shm_fd;
    void * ptr;
    
    // ******** shm_open() ***************////// 
    //   shm_open()- opening/creation of a shared memory segment referenced by a name
    //        1) A special file will appear in the file-system under “/dev/shm/” with the provided name.
    //        2) The special file represents a POSIX object and it is created for persistence 
    //        3) ftruncate(...) function resize to memory region to the correct size
    
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    
    if (shm_fd==1) 
    {
        printf("Shared memory segment failed\n");
        exit(1);
    }
    
    ftruncate(shm_fd, sizeof(message));
    
    // ******** shm_open() end ***************////// 
    
    // ******** mmap() ***************////// 
    // ******** mmap() – mapping of the memory segment referenced by the file descriptor returned by shm_open() ***************////// 
    
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    if (ptr == MAP_FAILED) 
    {
        printf("Map failed\n");
        return 1;
        
    }
    /* Write into the memory segment */
    for (i = 0; i < strlen(*message); ++i) 
    {
        sprintf(ptr, "%s", message[i]);
        ptr += strlen(message[i]);
        
    }
    // ******** munmap() ***************////// 
    // ******** munmap() – unmapping of the memory segment ***************////// 
    munmap(ptr, SIZE);
    
    return 0;
}