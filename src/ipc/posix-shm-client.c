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
    int i, shm_fd;
    void * ptr;
    
    // ******** shm_open() ***************////// 
    //   shm_open()- opening/creation of a shared memory segment referenced by a name
    //        1) A special file will appear in the file-system under “/dev/shm/” with the provided name.
    //        2) The special file represents a POSIX object and it is created for persistence 
    //        3) ftruncate(...) function resize to memory region to the correct size
    
    shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    
    if (shm_fd==1) 
    {
        printf("Shared memory segment failed\n");
        exit(1);
    }
    
    
    // ******** mmap() ***************////// 
    // ******** mmap() – mapping of the memory segment referenced by the file descriptor returned by shm_open() ***************////// 
    
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    
    if (ptr == MAP_FAILED) 
    {
        printf("Map failed\n");
        return 1;
        
    }
    /* Read into the memory segment */
    printf("%s \n ", (char *) ptr);
    
    //  shm_unlink()
    //  shm_unlink() – removal of shared memory segment object if nobody is referencing it
    
    if (shm_unlink(shm_name) ==1) 
    {
       printf("Error removing %s\n", shm_name);
       exit(1);
       
    }

    return 0;
}