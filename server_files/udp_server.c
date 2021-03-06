/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define MAXFILESIZE 1024 * 25
//25 used since size of largest file is 25kb, 1024 bytes in 1kb

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char **argv)
{
  int sockfd;                      /* socket */
  int portno;                      /* port to listen on */
  int clientlen;                   /* byte size of client's address */
  struct sockaddr_in serveraddr;   /* server's addr */
  struct sockaddr_in clientaddr;   /* client addr */
  struct hostent *hostp;           /* client host info */
  char receievedFile[MAXFILESIZE]; /* message buf */
  char sendFileBuffer[MAXFILESIZE];
  char lsContentsBuffer[MAXFILESIZE];
  char receivedCommand[10];
  char statusMessage[100];
  char filename[10];
  char *hostaddrp;    /* dotted decimal host addr string */
  int optval;         /* flag value for setsockopt */
  int bytesReceieved; /* message byte size */
  int bytesSent;
  int fileSize;
  int removeStatus;
  FILE *transmitFile;
  size_t bytesWritten;

  /* 
   * check command line arguments 
   */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket
     UDP socket with IPv4 internet protoclol, connectionless,  
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sockfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
             (const void *)&optval, sizeof(int));

  /*
   * build the server's Internet address
     stored in serveraddr struct, set each attribute to correct values
     htonl and htons converts to correct network byte order
   */
  bzero((char *)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *)&serveraddr,
           sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);

  printf("File transfer server running on port: %d\n\n", portno);
  printf("Waiting for message from a client...\n");

  while (1)
  {
    /*
     * recvfrom: receive a UDP datagram from a client
     */

    bzero(receievedFile, MAXFILESIZE);

    bytesReceieved = recvfrom(sockfd, receivedCommand, 10, 0, (struct sockaddr *)&clientaddr, &clientlen); //receiving first message from client which is which command it's using

    if (bytesReceieved < 0)
      error("ERROR in recvfrom");

    //gethostbyaddr: get information about client that send datagram, initially receive info from client in first recvfrom that fill out structs

    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                          sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");

    //
    //
    //
    //
    //
    //
    //
    //handing of command-dependent response
    //
    //
    //
    //
    //
    //
    //
    //

    //PUT
    if (strcmp(receivedCommand, "put") == 0)
    {

      printf("Command recieved: put\n");
      recvfrom(sockfd, filename, 10, 0, (struct sockaddr *)&clientaddr, &clientlen);                                //first receive to know filename for created file
      bytesReceieved = recvfrom(sockfd, receievedFile, MAXFILESIZE, 0, (struct sockaddr *)&clientaddr, &clientlen); // second is receiving bytes of actual file

      printf("Server received datagram(s) from %s (%s), containing %d bytes\n",
             hostp->h_name, hostaddrp, bytesReceieved);
      printf("Storing into file on server...\n");

      transmitFile = fopen(filename, "w+b");
      bytesWritten = fwrite(receievedFile, bytesReceieved, 1, transmitFile); //bytes written is really elements written
      fclose(transmitFile);

      bzero(statusMessage, 50);

      if (bytesWritten == 1) //elements written is indeed 1
      {
        printf("Success!\n");
        strcpy(statusMessage, "Success");
      }

      else
      {
        strcpy(statusMessage, "Fail");
      }

      //sending back status of operation
      bytesSent = sendto(sockfd, statusMessage, 50, 0,
                         (struct sockaddr *)&clientaddr, clientlen);
      if (bytesSent < 0)
        error("ERROR in sendto");
    }

    //GET
    else if (strcmp(receivedCommand, "get") == 0)
    {
      printf("Command recieved: get\n");
      bzero(filename, 10);
      recvfrom(sockfd, filename, 10, 0, (struct sockaddr *)&clientaddr, &clientlen); //first receive to know what the name of the file wanted is

      transmitFile = fopen(filename, "r");
      if (!transmitFile)
      {
        printf("Unable to find or open file given.\n");
        sendto(sockfd, statusMessage, 1, 0, (struct sockaddr *)&clientaddr, clientlen);
        return 1;
      }

      fseek(transmitFile, 0L, SEEK_END);
      fileSize = ftell(transmitFile);
      fseek(transmitFile, 0, SEEK_SET);
      // printf("File size: %d\n", fileSize);

      fread(sendFileBuffer, fileSize, 1, transmitFile);

      fclose(transmitFile);

      /*
      process: get filename of the file on the server the client wants, make sure the file is located here, get bytes into a buffer, get num bytes of file with fseek and ftell, send exactly that 
      much back to user. 
      */

      bytesSent = sendto(sockfd, sendFileBuffer, fileSize, 0, (struct sockaddr *)&clientaddr, clientlen);

      if (bytesSent < 0)
        error("ERROR in sendto");

      printf("Bytes sent to client: %d\n", bytesSent);
    }

    //DELETE
    else if (strcmp(receivedCommand, "delete") == 0)
    {
      printf("Command received: delete\n");
      bzero(filename, 10);
      recvfrom(sockfd, filename, 10, 0, (struct sockaddr *)&clientaddr, &clientlen); //receive to know what the name of the file wanted is
      printf("Filename: %s\n", filename);
      removeStatus = remove(filename);
      //handle whether removing was successful or not
      if (removeStatus == 0)
      {
        bzero(statusMessage, 100);
        strcpy(statusMessage, "File removed from server.");
        bytesSent = sendto(sockfd, statusMessage, 50, 0,
                           (struct sockaddr *)&clientaddr, clientlen);
        printf("File successfully removed from server\n");
      }
      else
      {
        bzero(statusMessage, 100);
        strcpy(statusMessage, "Failed to remove file from server. Either file does not exist or it is not the correct filename");
        bytesSent = sendto(sockfd, statusMessage, strlen(statusMessage), 0,
                           (struct sockaddr *)&clientaddr, clientlen);
        printf("%s\n", statusMessage);
      }
    }

    //LS
    else if (strcmp(receivedCommand, "ls") == 0)
    {
      printf("Command recieved: ls\n");
      printf("Printing all files on server:\n");

      //need <dirent.h> headr for looking into current directory
      struct dirent *directoryEntry;
      DIR *dr = opendir(".");
      if (dr == NULL)
      {
        printf("Error: Cannot open current directory\n");
        return 1;
      }
      while ((directoryEntry = readdir(dr)) != NULL)
      {
        sprintf(lsContentsBuffer + strlen(lsContentsBuffer), "%s", directoryEntry->d_name);
        sprintf(lsContentsBuffer + strlen(lsContentsBuffer), "\n");
      }

      closedir(dr);
    }

      /*
    using library for handling of viewing files in directory, get list of this and append to the buffer we'll send back to the user.
    */

    sendto(sockfd, lsContentsBuffer, strlen(lsContentsBuffer), 0, (struct sockaddr *)&clientaddr, clientlen);
    bzero(lsContentsBuffer, strlen(lsContentsBuffer));
    printf("\nWaiting for more messages from a client...\n");
  }
}
