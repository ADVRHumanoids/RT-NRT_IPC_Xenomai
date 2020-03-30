#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <rtdm/ipc.h>

#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>


#define SHMOBJ_PATH  "/shm_XBOT"
#define SEM_PATH     "/sem_XBOT"
#define XDDP_PORT_LABEL  "xddp-demo"
#define N_JOINT 10

pthread_t svtid, cltid,nrt;


struct shared_data 
{
    
    double q[N_JOINT];
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


void shutdownSIG(int sig __attribute__((unused)))
{
    main_loop = 0;
}

static void fail(const char *reason)
{
        perror(reason);
        exit(EXIT_FAILURE);
}
static void *HAL(void *arg)
{
    int shared_seg_size = (1 * sizeof(struct shared_data));
   
    
    int shmfd  = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    ftruncate(shmfd, shared_seg_size);
    struct shared_data * shared_msg = (struct shared_data *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED,shmfd, 0);
    
    struct timespec ts;
    struct timeval tv;
    
    
    sem_t * sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
    sem_init(sem_id,0,1);

    
    struct shared_data out_msg,in_msg;

    for(int i=0;i<N_JOINT;i++)
        out_msg.q[i]=0;

    while(main_loop)
    {
    
        sem_wait(sem_id);

        memcpy(shared_msg, &out_msg, sizeof(struct shared_data));
        
        sem_post(sem_id);


        ts.tv_sec = 0;
        ts.tv_nsec = 500000000; /* 500 ms */
        clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);

        sem_wait(sem_id);
        
        memcpy(&in_msg, shared_msg, sizeof(struct shared_data));
        
        
        out_msg=in_msg;
        
        for(int i=0;i<N_JOINT;i++)
            out_msg.q[i]++;
       
        sem_post(sem_id);
        


     }
    
    shm_unlink(SHMOBJ_PATH);

    sem_close(sem_id);
    
    sem_unlink(SEM_PATH);
    
    
    return NULL;
}
static void *plugin_handler(void *arg)
{
    struct shared_data in_msg,out_msg;
    struct rtipc_port_label plabel;
    struct sockaddr_ipc saddr;
    int ret, s, n = 0;
    socklen_t addrlen;
    struct timespec ts;
    struct timeval tv;
    
    /****************** SHARED MEMORY ******************* */
    
    int shared_seg_size = (1 * sizeof(struct shared_data));
    //int shmfd  = shm_open(SHMOBJ_PATH, O_RDWR, 0666);
    int shmfd  = shm_open(SHMOBJ_PATH, O_RDWR, S_IRWXU | S_IRWXG);
    struct shared_data * shared_msg = (struct shared_data *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED,shmfd, 0);
    sem_t * sem_id = sem_open(SEM_PATH, 0);
    sem_init(sem_id,0,0);
    /****************** SHARED MEMORY ******************* */
    
    
    /****************** SOCKECT ******************* */

    s = socket(AF_RTIPC, SOCK_DGRAM, IPCPROTO_XDDP);

    if (s < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    strcpy(plabel.label, XDDP_PORT_LABEL);
    ret = setsockopt(s, SOL_XDDP, XDDP_LABEL,
                        &plabel, sizeof(plabel));
    if (ret)
            fail("setsockopt");
    
    /*
    * Set a port label. This name will be used to find the peer
    * when connecting, instead of the port number.
    */

    memset(&saddr, 0, sizeof(saddr));
    saddr.sipc_family = AF_RTIPC;
    saddr.sipc_port = -1;   /* Tell XDDP to search by label. */
    ret = bind(s, (struct sockaddr *)&saddr, sizeof(saddr));
    if (ret)
        fail("bind");

    /*
        * We succeeded in making the port our default destination
        * address by using its label, but we don't know its actual
        * port number yet. Use getpeername() to retrieve it.
        */
    addrlen = sizeof(saddr);
    ret = getpeername(s, (struct sockaddr *)&saddr, &addrlen);
    if (ret || addrlen != sizeof(saddr))
            fail("getpeername");
    printf("%s: NRT peer is reading from /dev/rtp%d\n",
            __FUNCTION__, saddr.sipc_port);
    
    
    /****************** SOCKECT ******************* */
    while(main_loop)
    {
        
        /****************** SHARED MEMORY ******************* */
        sem_wait(sem_id);

        memcpy(&in_msg, shared_msg, sizeof(struct shared_data));
        
        sem_post(sem_id);

        /****************** SHARED MEMORY ******************* */

        /****************** SOCKECT ******************* */
        ret = sendto(s, &in_msg, sizeof(struct shared_data), 0, NULL, 0);
        if (ret != sizeof(struct shared_data))
                fail("sendto");
        /*
            * We run in full real-time mode (i.e. primary mode),
            * so we have to let the system breathe between two
            * iterations.
            */
        ts.tv_sec = 0;
        ts.tv_nsec = 500000000; /* 500 ms */
        clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
        
        ret = recvfrom(s,&in_msg, sizeof(struct shared_data), 0, NULL, 0);
        if (ret <= 0)
            fail("recvfrom");
        
        printf("%s: \n", __FUNCTION__);
        for(int i=0;i<N_JOINT;i++)
            printf("Joint[%d]: %f ",i+1,in_msg.q[i]);
        printf("\n");
        /****************** SOCKECT ******************* */
        
        out_msg=in_msg;
        
        sem_wait(sem_id);
        
        memcpy(shared_msg, &out_msg, sizeof(struct shared_data));
        
        sem_post(sem_id);
        
        ts.tv_sec = 0;
        ts.tv_nsec = 500000000; /* 500 ms */
        clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
 

    }
    
    if (shm_unlink(SHMOBJ_PATH) ==1) 
        fail("Error removing the share memory");

    sem_close(sem_id);

    sem_unlink(SEM_PATH);

    return NULL;
}

static void *comm_handler(void *arg)
{
        struct shared_data in_msg;
        char  *devname;
        int fd, ret;
        if (asprintf(&devname,
                     "/proc/xenomai/registry/rtipc/xddp/%s",
                     XDDP_PORT_LABEL) < 0)
                fail("asprintf");
        fd = open(devname, O_RDWR);
        free(devname);
        if (fd < 0)
                fail("open");
       
        
        while(main_loop)
        {
                /* Get the next message from realtime_thread2. */
                ret = read(fd, &in_msg, sizeof(struct shared_data));
                if (ret <= 0)
                        fail("read");
                
               for(int i=0;i<N_JOINT;i++)
                    in_msg.q[i]++;
               
                /* Relay the message to realtime_thread1. */
                ret = write(fd, &in_msg, ret);
                if (ret <= 0)
                      fail("write");
               
        }
        return NULL;
}

int main(int argc, char **argv)
{
        struct sched_param svparam = {.sched_priority = 71 };
        struct sched_param clparam = {.sched_priority = 70 };
        pthread_attr_t svattr, clattr,regattr;
        sigset_t set;
        int sig;
        
        main_common(shutdownSIG);
        
        sigemptyset(&set);
        sigaddset(&set, SIGINT);
        sigaddset(&set, SIGTERM);
        sigaddset(&set, SIGHUP);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
        pthread_attr_init(&svattr);
        pthread_attr_setdetachstate(&svattr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setinheritsched(&svattr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&svattr, SCHED_FIFO);
        pthread_attr_setschedparam(&svattr, &svparam);
        errno = pthread_create(&svtid, &svattr, &HAL, NULL);
        if (errno)
                fail("pthread_create");
        pthread_attr_init(&clattr);
        pthread_attr_setdetachstate(&clattr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setinheritsched(&clattr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&clattr, SCHED_FIFO);
        pthread_attr_setschedparam(&clattr, &clparam);
        errno = pthread_create(&cltid, &clattr, &plugin_handler, NULL);
        if (errno)
                fail("pthread_create");
        
        pthread_attr_init(&regattr);
        pthread_attr_setdetachstate(&regattr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setinheritsched(&regattr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&regattr, SCHED_OTHER);
        errno = pthread_create(&nrt, &regattr, &comm_handler, NULL);
        if (errno)
                fail("pthread_create");
        
        sigwait(&set, &sig);
        pthread_cancel(svtid);
        pthread_cancel(cltid);
         pthread_cancel(nrt);
        pthread_join(svtid, NULL);
        pthread_join(cltid, NULL);
        pthread_join(nrt, NULL);
        return 0;
}