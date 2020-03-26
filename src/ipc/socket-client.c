#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DATA "The sea is calm tonight, the tide is full . . ."

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in name;
    struct hostent *hp, *gethostbyname();

    /* Create socket on which to send. */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("opening datagram socket");
        exit(1);
    }
    /*
     * Construct name, with no wildcards, of the socket to 
     * send to. Gethostbyname() returns a structure including
     * the network address of the specified host. The port
     * number is taken from the command line.
     */
     hp = gethostbyname(argv[1]);  ////// NOTE insert: localhost //////
     if (hp == 0) {
         fprintf(stderr, "%s: unknown host\n", argv[1]);
         exit(2);
     }
     memcpy(&name.sin_addr, hp->h_addr, hp->h_length);
     name.sin_family = AF_INET;
     name.sin_port = htons(atoi(argv[2]));  ////// NOTE insert: PORT retrieved by the server //////

     /* Send message. */
     if (sendto(sock, DATA, sizeof(DATA), 0,(struct sockaddr *)&name, sizeof(name)) < 0)
         perror("sending datagram message");
     close(sock);
     return 0;
}