#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include <semaphore.h>
#define SHMOBJ_PATH  "/shm_AOS"
#define SEM_PATH     "/sem_AOS"

struct shared_data 
{
    char var1[10];
    int var2;
};

static void set_signal_handler ( __sighandler_t sig_handler ) {
    signal ( SIGINT, sig_handler );
    signal ( SIGKILL, sig_handler );
}

void main_common ( __sighandler_t sig_handler ) {
    int ret;

    set_signal_handler ( sig_handler );
    
    return;
}


static int main_loop = 1;


void shutdown(int sig __attribute__((unused)))
{
    main_loop = 0;
}



int main (int argc, char *argv[]) 
{

    int shared_seg_size = (1 * sizeof(struct shared_data));
    
        // ******** shm_open() ***************////// 
    //   shm_open()- opening/creation of a shared memory segment referenced by a name
    //        1) A special file will appear in the file-system under “/dev/shm/” with the provided name.
    //        2) The special file represents a POSIX object and it is created for persistence 
    //        3) ftruncate(...) function resize to memory region to the correct size
    
    int shmfd  = shm_open(SHMOBJ_PATH, O_RDONLY, 0666);

    struct shared_data * shared_msg = (struct shared_data *)mmap(NULL, shared_seg_size, PROT_READ, MAP_SHARED, shmfd, 0);
    
    // sem_open()
    // sem_open() – opening/creation of a named semaphore
    //             -Useful for synchronization among unrelated processes
    
    sem_t * sem_id = sem_open(SEM_PATH, 0);
    struct shared_data in_msg;
    
    main_common(shutdown);
    
    while(main_loop)
    {
        printf ("Waiting.....\n");
        sleep(5);
        // sem_wait() 
        // sem_wait() – Decrement the counter and lock if counter = 0
        //              Initial counter value can be set to > 1
        sem_wait(sem_id);
        
        /* Update shared data */
        memcpy(&in_msg, shared_msg, sizeof(struct shared_data));
        
        // sem_post()
        // sem_post() – Increment the count and unlock the critical section if counter > 0
        sem_post(sem_id);
        
        /* Process data... */
        printf("Name: %s ,Age: %d\n",in_msg.var1,in_msg.var2);
        
    } 
    
    //  shm_unlink()
    //  shm_unlink() – removal of shared memory segment object if nobody is referencing it
    shm_unlink(SHMOBJ_PATH);
    
    //sem_close()
    //sem_close() – Close all the references to the named semaphore
    sem_close(sem_id);
    
    //sem_unlink
    //sem_unlink() – Destroy semaphore object 
    //               if all the references have been closed
    sem_unlink(SEM_PATH);


    return 0;
}