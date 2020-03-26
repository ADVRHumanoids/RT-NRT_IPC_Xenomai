#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define CLIENT_SOCK_FILE "client.sock"
#define SERVER_SOCK_FILE "server.sock"

int main() {
    int fd;
    struct sockaddr_un addr;
    int ret;
    char buff[8192];
    struct sockaddr_un from;
    int ok = 1;
    int len;
    
    //////********* UNIX domain  *********//////////
    ///// UNIX domain sockets are a method by which processes on the same host can communicate. 
    /////Communication is bidirectional with stream sockets and unidirectional with datagram sockets.
    
    // i.e fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    /// PF= Protocol family: Unix domain sockets  PF_UNIX=AF_UNIX
    //NOTE: SOCK_DGRAM socket datagram type communication. other type stream
    if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        ok = 0;
    }

    if (ok) {
        memset(&addr, 0, sizeof(addr));
        ///  AF=Address familu UNIX
        addr.sun_family = AF_UNIX;
        ////// path name = client.sock
        strcpy(addr.sun_path, CLIENT_SOCK_FILE);
        // unlink() - delete name and if it's possible the file which is referring for by the name.
        unlink(CLIENT_SOCK_FILE);
        //// int bind (int fd, const struct sockaddr *addr, int addrlen)
        ////  link the socket using the file descriptor a local or a network Address, memorized into addr.
        
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind");
            ok = 0;
        }
    }

    if (ok) {
        memset(&addr, 0, sizeof(addr));
        ///  AF=Address familu UNIX
        addr.sun_family = AF_UNIX;
        ////// path name = server.sock
        strcpy(addr.sun_path, SERVER_SOCK_FILE);
        /// int connect (int fd, const struct sockaddr *addr,int addrlen), 
        ///try to connet to a specific socket created by the client, using the address specified into the addr structure
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("connect");
            ok = 0;
        }
    }

    if (ok) {
        strcpy (buff, "iccExchangeAPDU");
        //// ssize_t send(int sockfd, const void *buf, size_t len, int flags);  you could use the write function since it's possible use the generic file descriptor
        if (send(fd, buff, strlen(buff)+1, 0) == -1) {
            perror("send");
            ok = 0;
        }
        printf ("sent iccExchangeAPDU\n");
    }

    if (ok) {
        //// ssize_t recv(int sockfd, void *buf, size_t len, int flags);  you could use the read function since it's possible use the generic file descriptor
        if ((len = recv(fd, buff, 8192, 0)) < 0) {
            perror("recv");
            ok = 0;
        }
        printf ("receive %d %s\n", len, buff);
    }

    if (fd >= 0) {
        // close file descriptor
        close(fd);
    }
    // unlink() - delete name and if it's possible the file which is referring for by the name.
    unlink (CLIENT_SOCK_FILE);
    return 0;
}