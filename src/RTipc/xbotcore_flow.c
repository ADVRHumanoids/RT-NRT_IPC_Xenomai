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
#define XDDP_PORT_LABEL  "xddp-demo"
pthread_t svtid, cltid,nrt;


static const char *msg[]={""};

static void fail(const char *reason)
{
        perror(reason);
        exit(EXIT_FAILURE);
}
static void *HAL(void *arg)
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
        fail("Shared memory segment failed\n");
    
    ftruncate(shm_fd, sizeof(message));
    
    // ******** shm_open() end ***************////// 
    
    // ******** mmap() ***************////// 
    // ******** mmap() – mapping of the memory segment referenced by the file descriptor returned by shm_open() ***************////// 
    
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    if (ptr == MAP_FAILED) 
        fail("Map failed\n");

    /* Write into the memory segment */
    for (i = 0; i < strlen(*message); ++i) 
    {
        sprintf(ptr, "%s", message[i]);
        ptr += strlen(message[i]);
        
    }
    // ******** munmap() ***************////// 
    // ******** munmap() – unmapping of the memory segment ***************////// 
    munmap(ptr, SIZE);
    return NULL;
}
static void *plugin_handler(void *arg)
{
    const char * shm_name  = "/AOS";
    const int SIZE = 4096;
    int i, shm_fd;
    void * ptr;
    char buf[128];
    
    struct rtipc_port_label plabel;
    struct sockaddr_ipc saddr;
    int ret, s, n = 0, len;
    struct timespec ts;
    struct timeval tv;
    socklen_t addrlen;
    
    // ******** shm_open() ***************////// 
    //   shm_open()- opening/creation of a shared memory segment referenced by a name
    //        1) A special file will appear in the file-system under “/dev/shm/” with the provided name.
    //        2) The special file represents a POSIX object and it is created for persistence 
    //        3) ftruncate(...) function resize to memory region to the correct size
    
    shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    
    if (shm_fd==1) 
        fail("Shared memory segment failed\n");
    
    
    // ******** mmap() ***************////// 
    // ******** mmap() – mapping of the memory segment referenced by the file descriptor returned by shm_open() ***************////// 
    
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    
    if (ptr == MAP_FAILED) 
      fail("Map failed\n");

    /* Read into the memory segment */
    printf("%s \n ", (char *) ptr);
    
    *msg=(char *)ptr;
    
    s = socket(AF_RTIPC, SOCK_DGRAM, IPCPROTO_XDDP);
    if (s < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
    }

    /*
        * Set a port label. This name will be used to find the peer
        * when connecting, instead of the port number.
        */
    strcpy(plabel.label, XDDP_PORT_LABEL);
    ret = setsockopt(s, SOL_XDDP, XDDP_LABEL,
                        &plabel, sizeof(plabel));
    if (ret)
            fail("setsockopt");
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
    for (;;) {
            len = strlen(msg[n]);
            /*
                * Send a datagram to the NRT endpoint via the proxy.
                * We may pass a NULL destination address, since the
                * socket was successfully assigned the proper default
                * address via connect(2).
                */
            ret = sendto(s, msg[n], len, 0, NULL, 0);
            if (ret != len)
                    fail("sendto");
            printf("%s: sent %d bytes, \"%.*s\"\n",
                    __FUNCTION__, ret, ret, msg[n]);
            n = (n + 1) % (sizeof(msg) / sizeof(msg[0]));
            /*
                * We run in full real-time mode (i.e. primary mode),
                * so we have to let the system breathe between two
                * iterations.
                */
            ts.tv_sec = 0;
            ts.tv_nsec = 500000000; /* 500 ms */
            clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
            
             ret = recvfrom(s, buf, sizeof(buf), 0, NULL, 0);
             if (ret <= 0)
                 fail("recvfrom");
             printf("%s: \"%.*s\" relayed by peer\n", __FUNCTION__, ret, buf);
    }
    
    //  shm_unlink()
    //  shm_unlink() – removal of shared memory segment object if nobody is referencing it
    
    if (shm_unlink(shm_name) ==1) 
       fail("Error removing the share memory");

    return NULL;
}

static void *comm_handler(void *arg)
{
        char buf[128], *devname;
        int fd, ret;
        if (asprintf(&devname,
                     "/proc/xenomai/registry/rtipc/xddp/%s",
                     XDDP_PORT_LABEL) < 0)
                fail("asprintf");
        fd = open(devname, O_RDWR);
        free(devname);
        if (fd < 0)
                fail("open");
        for (;;) {
                /* Get the next message from realtime_thread2. */
                ret = read(fd, buf, sizeof(buf));
                if (ret <= 0)
                        fail("read");
                printf("%s: received %d bytes, \"%.*s\"\n",
                    __FUNCTION__, ret, ret, buf);
                /* Relay the message to realtime_thread1. */
                ret = write(fd, buf, ret);
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