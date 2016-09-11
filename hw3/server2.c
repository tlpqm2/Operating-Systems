/************************************************************************/
/*   PROGRAM NAME: server2.c  (works with client.c)                     */
/*                                                                      */
/*   Server creates a socket to listen for the connection from Client   */
/*   When the communication established, Server echoes data from Client */
/*   and writes them back.                                              */
/*                                                                      */
/*   Using socket() to create an endpoint for communication. It         */
/*   returns socket descriptor. Stream socket (SOCK_STREAM) is used here*/
/*   as opposed to a Datagram Socket (SOCK_DGRAM)                       */  
/*   Using bind() to bind/assign a name to an unnamed socket.           */
/*   Using listen() to listen for connections on a socket.              */
/*   Using accept() to accept a connection on a socket. It returns      */
/*   the descriptor for the accepted socket.                            */
/*                                                                      */
/*   To run this program, first compile the server_ex.c and run it      */
/*   on a server machine. Then run the client program on another        */
/*   machine.                                                           */
/*                                                                      */
/*   LINUX:      gcc -std=c99 -pthread -o server2  server2.c -lnsl      */
/*                                                                      */
/************************************************************************/

/* PROGRAMMED BY: Tyler Patton and Sawyer McLane 
   SPRING 2016 CS3800 Section B                  */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 9976   /* define a server port number */
#define MAX_THREADS 3		/* max number of concurrent users/threads */
#define BUFFER_SIZE 512		/* maximum message size */
#define MAX_NAME_SIZE 20	/* largest username size */
#define UNUSED_SOCKET -2	/* "magic number" signifying unused socket slot in socket array */
#define EXIT_CODE "404"


struct threadData
{
	//allows passage of thead Index and Socket to handler function (also username)
    int threadIndex;
    int threadSocket;
    char username[MAX_NAME_SIZE];
};
typedef struct threadData threadData;

void sig_handler(int sig);
void *clientHandler(void * myData);
void writeAll(char buffer[], int socketID);
int sockets[MAX_THREADS];
int global_sd, threadCount = 0;
struct sockaddr_in global_server_addr;
pthread_mutex_t threadLock = PTHREAD_MUTEX_INITIALIZER;
pthread_t threadArray[MAX_THREADS];

int main()
{
    int sd, ns, k;
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
    struct sockaddr_in client_addr = { AF_INET };
    int client_len = sizeof( client_addr );
    char buf[BUFFER_SIZE], *host;
	global_server_addr = server_addr;
    //pthread_mutext_init(&threadLock, NULL);

	//initialize the socket array to be available
    for(int i = 0; i < MAX_THREADS; i++)
    {
        sockets[i] = UNUSED_SOCKET;
    }

    /* create a stream socket */
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
      perror( "server: socket failed" );
      exit( 1 );
    }
    
    /* bind the socket to an internet port */
    if( bind(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 )
    {
      perror( "server: bind failed" );
      exit( 1 );
    }
    global_sd = sd;
    /*
    THREADCOUNT OVERWRITTING STUFF (PROBABLY DONE)
    */
    /* listen for clients */
    if(listen( sd, 1 ) == -1)
    {
      perror( "server: listen failed" );
      exit( 1 );
    }

    printf("SERVER is listening for clients to establish a connection\n");
    signal(SIGINT, sig_handler);
    while(1)
    {
        while(threadCount < MAX_THREADS)
        {
            if((ns = accept(sd, (struct sockaddr*)&client_addr, &client_len ) ) == -1)
            {
                perror("server: accept failed");
                exit(1);
            }
            int tempThreadIndex = -1;
            for(int index = 0; index < MAX_THREADS; index++)
            {
            	//iterate over sockets
            	if(sockets[index] == UNUSED_SOCKET)
            	{
            		//if socket is free, take it
            		tempThreadIndex = index;
            	}
            }
            if(tempThreadIndex != -1) //if a socket was found
            {
	            threadData myData;
	            myData.threadIndex = tempThreadIndex;
	            myData.threadSocket = ns;
	            read(ns, myData.username, MAX_NAME_SIZE);
	            threadCount++;

	            sockets[tempThreadIndex] = ns;
	            pthread_mutex_lock(&threadLock);

	            if(pthread_create(&threadArray[threadCount], NULL, clientHandler, &myData))
	            {
	                printf("Failed to create thread\n");
	                pthread_mutex_unlock(&threadLock);
	                exit(0);
	            }  
            }
        }
    }
    
    return(0);
}

void sig_handler(int sig)
{
    pthread_mutex_lock(&threadLock);
    char msg[] = "\nServer will shut down in 10 seconds!\n";
    printf("%s\n", msg);
    writeAll(msg, -3);
    sleep(1);

    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(sockets[i] >= 0)
        {
          close(sockets[i]);
          sockets[i] = UNUSED_SOCKET;
          pthread_cancel(threadArray[i]);
          threadCount--;
        }
    }
    pthread_mutex_unlock(&threadLock);
    close(global_sd);
    exit(0);
}

void *clientHandler(void * myData)
{
    threadData thisThread = *(threadData*)myData;
    char buffer[BUFFER_SIZE + MAX_NAME_SIZE];
    char tempusername[BUFFER_SIZE];
    strcpy(tempusername, thisThread.username);
    strcat(tempusername, ": ");
    int k;

    pthread_mutex_unlock(&threadLock);
    printf("%s has just connected to the chat room!\n", thisThread.username);

    //data transfer on connected socket ns
    while( (k = read(thisThread.threadSocket, buffer, BUFFER_SIZE)) != 0)
    {    
        pthread_mutex_lock(&threadLock);
        if (strcmp(buffer, EXIT_CODE) == 0)
        {
        	//check to see if code is exiting
        	sockets[thisThread.threadIndex] = UNUSED_SOCKET;
        	printf("Thread %d has exited", thisThread.threadIndex);
        	pthread_cancel(threadArray[thisThread.threadIndex]);
        	close(thisThread.threadSocket);
        	break;
        }
        printf("%s: %s\n", thisThread.username, buffer );
        writeAll(strcat(tempusername, buffer), thisThread.threadSocket);
        pthread_mutex_unlock(&threadLock);
        strcpy(buffer, "");
        strcpy(tempusername, thisThread.username);
        strcat(tempusername, ": ");
    }
}

void writeAll(char buffer[], int socketID)
{
    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(sockets[i] >= 0 && sockets[i] != socketID)
        {
            write(sockets[i], buffer, BUFFER_SIZE);            
        }
    }
}