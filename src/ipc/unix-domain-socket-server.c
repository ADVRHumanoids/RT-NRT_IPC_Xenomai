#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define SERVER_SOCK_FILE "server.sock"
int main() {
    int fd;
    struct sockaddr_un addr;
    int ret;
    char buff[8192];
    struct sockaddr_un from;
    int ok = 1;
    int len;
    socklen_t fromlen = sizeof(from);
    
    //////********* UNIX domain  *********//////////
    ///// UNIX domain sockets are a method by which processes on the same host can communicate. 
    /////Communication is bidirectional with stream sockets and unidirectional with datagram sockets.
    
    // i.e fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    /// PF= Protocol family: Unix domain sockets  PF_UNIX=AF_UNIX
    //NOTE: SOCK_DGRAM socket datagram type communication.
    if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        ok = 0;
    }

    if (ok) {
        
        memset(&addr, 0, sizeof(addr));
        ///  AF=Address familu UNIX
        addr.sun_family = AF_UNIX;
        ////// path name = server.sock
        strcpy(addr.sun_path, SERVER_SOCK_FILE);
        // unlink() - delete name and if it's possible the file which is referring for by the name.
        unlink(SERVER_SOCK_FILE);
        //// int bind (int fd, const struct sockaddr *addr, int addrlen)
        ////  link the socket using the file descriptor a local or a network Address, memorized into addr.
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind");
            ok = 0;
        }
    }
    
    /// With the SOCK SOCK_DGRAM option it's possibile use the recvfrom function with more details.
    ////ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    while ((len = recvfrom(fd, buff, 8192, 0, (struct sockaddr *)&from, &fromlen)) > 0) {
        printf ("recvfrom: %s\n", buff);
        strcpy (buff, "transmit good!");
        /// With the SOCK SOCK_DGRAM option it's possibile use the sendto function with more details.
        /// ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,const struct sockaddr *dest_addr, socklen_t addrlen);
        ret = sendto(fd, buff, strlen(buff)+1, 0, (struct sockaddr *)&from, fromlen);
        if (ret < 0) {
            perror("sendto");
            break;
        }
    }
    

    if (fd >= 0) {
        close(fd);
    }

    return 0;
}