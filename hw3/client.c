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
/*   COMPILE:    gcc -std=c99 -pthread -o client client.c -lnsl         */ 
/*   TO RUN:     client  server-machine-name                            */ 
/*                                                                      */ 
/************************************************************************/ 

/* PROGRAMMED BY: Sawyer McLane and Tyler Patton
   SPRING 2016 CS3800 Section B                  */
 
#include <stdio.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  /* define socket */ 
#include <netinet/in.h>  /* define internet socket */ 
#include <netdb.h>       /* define internet socket */ 
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <strings.h>

#define SERVER_PORT 9976     /* define a server port number */ 
#define BUFFER_SIZE 512
#define MAX_NAME_SIZE 20
#define EXIT_CODE "404"

void client_sig_handler(int sig);
struct sockaddr_in global_server_addr;
int global_sd;
void *read_handle(void* arg);
void *write_handle(void* arg);

int main( int argc, char* argv[] ) 
{ 
  if( argc != 2 ) 
  { 
    printf( "Usage: %s hostname\n", argv[0] ); 
    exit(1); 
  }
  int sd; 
  struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) }; 
  global_server_addr = server_addr;
  char buf[BUFFER_SIZE], username[MAX_NAME_SIZE]; 
  struct hostent *hp; 
  pthread_t readThread, writeThread;

  signal(SIGINT, client_sig_handler); 
  
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
  global_sd = sd; 

  //Prompt for username
  printf("Please enter a username (max 20 characters)\n");
  gets(username);
  write(sd, username, MAX_NAME_SIZE); 
  printf("You are now connected to the server! Play nice.\n" ); 

  //Write thread
  if(pthread_create(&writeThread, NULL, write_handle, NULL))
  {
      printf("Failed to create thread\n");
      exit(0);
  }
  //Read thread 
  if(pthread_create(&readThread, NULL, read_handle, NULL))
  {
      printf("Failed to create thread\n");
      exit(0);
  }

  pthread_join(writeThread, NULL); //wait for user to enter exit message
  //unlink client address?
  close(sd); 
  return(0); 
} 

void client_sig_handler(int sig)
{
    printf("\nDon't do that! Please exit with any of the following commands:\n");
    printf(" \\exit, \\quit, \\part \n");
}

void *write_handle(void* arg)
{
  char buf[BUFFER_SIZE];
  char checkquit[] = "\\quit", checkpart[] = "\\part", checkexit[] = "\\exit", checkbuf[5];
  while(1) 
  { 
    gets(buf);
    strcpy(checkbuf, buf);
    if(strcmp(checkbuf, checkexit) == 0 || strcmp(checkbuf, checkquit) == 0 || strcmp(checkbuf, checkpart) == 0)
    {
      //Write parting message from server to other users?
      printf("You are now leaving the chat room\n");
      strcpy(checkbuf, EXIT_CODE);
      write(global_sd, checkbuf, BUFFER_SIZE);
      pthread_exit(0);
    }
    if(strcmp(buf, "") != 0);
    {
      write(global_sd, buf, BUFFER_SIZE);
      strcpy(buf, "");
    }
  }
}

void *read_handle(void* arg)
{
  int k;
  char buffer[BUFFER_SIZE];
  while(1)
  {
    while((k = read(global_sd, buffer, BUFFER_SIZE)) != 0)
    {    
        printf("%s\n", buffer);
        strcpy(buffer, "");
    }
  }
}