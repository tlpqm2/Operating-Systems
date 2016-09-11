/************************************************************************/ 
/*   PROGRAM NAME: client.c  (works with serverX.c)                     */ 
/*                                                                      */ 
/*   Client creates a socket to connect to Server.                      */ 
/*   When the communication established, Client writes data to server   */ 
/*   and echoes the response from Server.                               */ 
/*                                                                      */ 
/*   To run this program, first compile the server_ex.c and run it      */ 
/*   on a server machine. Then run the client program on another        */ 
/*   machine.                                                           */ 
/*                                                                      */ 
/*   COMPILE:    gcc -o client client.c -lnsl                           */ 
/*   TO RUN:     client  server-machine-name                            */ 
/*                                                                      */ 
/************************************************************************/ 
 
#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  /* define socket */ 
#include <netinet/in.h>  /* define internet socket */ 
#include <netdb.h>       /* define internet socket */ 
 
#define SERVER_PORT 9985     /* define a server port number */ 

void sig_handler(int sig);

int main( int argc, char* argv[] ) 
{ 
    int sd; 
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) }; 
    char buf[512], username[20]; 
    char checkquit[] = "\\quit", checkpart[] = "\\part", checkexit[] = "\\exit", checkbuf[5];
    struct hostent *hp; 

    signal(SIGINT, SIG_IGN); //Ignore cntrl signal sent
    if(signal(SIGINT, sig_handler) == SIG_ERR)
    {
        printf("Parent: Unable to create handler for SIGINT\n");
    }

    //Prompt for username
    printf("Please enter a username (max 20 characters)\n");
    gets(username);

    if( argc != 2 ) 
    { 
      printf( "Usage: %s hostname\n", argv[0] ); 
      exit(1); 
    } 
 
    /* get the host */ 
    if( ( hp = gethostbyname( argv[1] ) ) == NULL ) 
    { 
      printf( "%s: %s unknown host\n", argv[0], argv[1] ); 
      exit( 1 ); 
    } 
    bcopy( hp->h_addr_list[0], (char*)&server_addr.sin_addr, hp->h_length ); 
 
    /* create a socket */ 
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) 
    { 
      perror( "client: socket failed" ); 
      exit( 1 ); 
    } 
 
    /* connect a socket */ 
    if( connect( sd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) == -1 ) 
    { 
      perror( "client: connect FAILED:" ); 
      exit( 1 ); 
    } 
 
    printf("connect() successful! will send a message to server\n"); 
    printf("Input a string:\n" ); 
 
    while( gets(buf) != NULL) 
    { 
      strcpy(checkbuf, buf, 5);
      if(strcmp(checkbuf, checkexit) == 0 || strcmp(checkbuf, checkquit) == 0 || strcmp(checkbuf, checkpart) == 0)
      {
        //Write parting message from server to other users?
        printf("You are now leaving the chat room\n");
        break;
      }
      write(sd, buf, sizeof(buf)); 
      read(sd, buf, sizeof(buf)); 
      //printf("SERVER ECHOED: %s\n", buf);
    } 
 
    close(sd); 
    return(0); 
} 

void sig_handler(int sig)
{
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
    printf("Don't do that! Please exit with any of the following commands:\n");
    printf(" \\exit, \\quit, \\part \n");
}