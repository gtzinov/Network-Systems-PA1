/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSIZE 1024
#define FILENAMESIZE 1024
#define MAXFILESIZE 1024 * 25
//25 used since largest file is 22kb

/* 
 * error - wrapper for perror
 */
void error(char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char **argv)
{
  int sockfd, portno;
  int fileSize;
  int numBytesSent;
  int numBytesReceived;
  int serverlen;
  struct sockaddr_in serveraddr;
  struct hostent *server;
  char *hostname;
  char buf[BUFSIZE];
  char filename[FILENAMESIZE];
  char serverResponse[MAXFILESIZE];
  char fileBuffer[MAXFILESIZE];
  char commandBuffer[10];
  int removeStatus;
  FILE *transmitFile;

  /* check command line arguments */
  if (argc != 3)
  {
    fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
    exit(0);
  }
  hostname = argv[1];
  portno = atoi(argv[2]);

  /* socket: create the socket */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL)
  {
    fprintf(stderr, "ERROR, no such host as %s\n", hostname);
    exit(0);
  }

  /* build the server's Internet address */
  bzero((char *)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
        (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portno);

  /* get a command from the user */
  bzero(buf, BUFSIZE);
  printf("Please enter command: ");
  fgets(buf, BUFSIZE, stdin);

  //!!!
  //ALL COMMAND IMPLEMENTATIONS HERE
  //!!!

  //PUT
  if (strcmp(buf, "put\n") == 0)
  {
    strcpy(commandBuffer, "put"); //to send in a message for server to know action

    printf("Which file would you like to put on the server?\n");
    fgets(filename, FILENAMESIZE, stdin);
    filename[strcspn(filename, "\n")] = 0;

    //opening and reading file action
    transmitFile = fopen(filename, "r");
    if (!transmitFile)
    {
      printf("Unable to find or open file given.\n");
      return;
    }

    // fseek(transmitFile, 0L, SEEK_END);
    // fileSize = ftell(transmitFile);
    // printf("File size: %d\n", fileSize);
    fread(fileBuffer, MAXFILESIZE, 1, transmitFile);

    //sending messages to server
    serverlen = sizeof(serveraddr);
    sendto(sockfd, commandBuffer, 10, 0, &serveraddr, serverlen);                      //first send one message of which command
    sendto(sockfd, filename, 10, 0, &serveraddr, serverlen);                           //second message is sending filename information to server so it can create file
    numBytesSent = sendto(sockfd, fileBuffer, MAXFILESIZE, 0, &serveraddr, serverlen); //last message is contents of file
    if (numBytesSent < 0)
      error("ERROR in sendto");

    //server response
    numBytesReceived = recvfrom(sockfd, serverResponse, MAXFILESIZE, 0, &serveraddr, &serverlen);
    if (numBytesReceived < 0)
      error("ERROR in recvfrom");
    printf("Response from server: %s\n", serverResponse);
    if (strcmp(serverResponse, "Success") == 0)
    {
      printf("File put on server and removed from local directory.\n");
      removeStatus = remove(filename);
    }
    else
    {
      printf("File write to server failed. File is still in local directory.\n");
    }
    return 0;
  }

  //GET
  else if (strcmp(buf, "get\n") == 0)
  {
    printf("Which file would you like to get from the server?\n");
    fgets(filename, FILENAMESIZE, stdin);
    //finish!
  }

  //DELETE
  else if (strcmp(buf, "delete\n") == 0)
  {
    printf("Which file would you like to delete from the server?\n");
    fgets(filename, FILENAMESIZE, stdin);
    //finish!
  }

  //LS
  else if (strcmp(buf, "ls\n") == 0)
  {
    printf("Here are all of the files on the server:");
    //finish!
  }

  //EXIT
  else if (strcmp(buf, "exit\n") == 0)
  {
    printf("Exiting program\n");
    //finish!
  }

  else
  {
    printf("Please enter valid command.\n");
    return;
  }

  /* print the server's reply */
}
