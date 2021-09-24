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
------

TO-DO: 
-ls command
-edge case handling with invalid commands, file names
-review difference in sending something in sendto and recv parameters for server vs client
-understand each function
-look at assignment requirements for documentation

------
*/

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
  char statusMessage[100];
  char fileBuffer[MAXFILESIZE];
  char commandBuffer[10];
  int removeStatus;
  FILE *transmitFile;
  int bytesWritten;
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
      return 1;
    }

    fseek(transmitFile, 0L, SEEK_END);
    fileSize = ftell(transmitFile);
    fseek(transmitFile, 0, SEEK_SET);
    // printf("File size: %d\n", fileSize);

    fread(fileBuffer, fileSize, 1, transmitFile);

    fclose(transmitFile);

    //open file you want to send, write bytes into buffer, then send buffer.
    //since we're not worrying about packet loss, can just send it one big buffer.
    //otherwise would be more ideal to send into smaller packets for reliability, so that if we need to resend a packet it doesn't have to be massive packet

    //sending messages to server
    serverlen = sizeof(serveraddr);
    sendto(sockfd, commandBuffer, 10, 0, &serveraddr, serverlen);                   //first send one message of which command
    sendto(sockfd, filename, 10, 0, &serveraddr, serverlen);                        //second message is sending filename information to server so it can create file
    numBytesSent = sendto(sockfd, fileBuffer, fileSize, 0, &serveraddr, serverlen); //last message is contents of file
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
    strcpy(commandBuffer, "get"); //to send in a message for server to know action

    printf("Which file would you like to get from the server?\n");
    fgets(filename, FILENAMESIZE, stdin);
    filename[strcspn(filename, "\n")] = 0;

    serverlen = sizeof(serveraddr);
    sendto(sockfd, commandBuffer, 10, 0, &serveraddr, serverlen); //sending command type
    sendto(sockfd, filename, 10, 0, &serveraddr, serverlen);      // sending filename information to server so it can know which file to return

    numBytesReceived = recvfrom(sockfd, serverResponse, MAXFILESIZE, 0, &serveraddr, &serverlen);

    if (numBytesReceived == 1)
    {
      //created system on both ends to detect when file doesn't exist on server: server just sends one byte back and client knows how to detect and read this
      printf("File not found on server\n");
      return 1;
    }

    transmitFile = fopen(filename, "w+b");
    bytesWritten = fwrite(serverResponse, numBytesReceived, 1, transmitFile);
    fclose(transmitFile);
    //received bytes, write into file on local side

    bzero(statusMessage, 50);

    if (bytesWritten == 1)
    {
      printf("Success!\n");
      strcpy(statusMessage, "Success");
    }

    else
    {
      strcpy(statusMessage, "Fail");
    }
  }

  //DELETE
  else if (strcmp(buf, "delete\n") == 0)
  {
    strcpy(commandBuffer, "delete"); //to send in a message for server to know action

    printf("Which file would you like to delete from the server?\n");
    fgets(filename, FILENAMESIZE, stdin);
    filename[strcspn(filename, "\n")] = 0;

    serverlen = sizeof(serveraddr);
    sendto(sockfd, commandBuffer, 10, 0, &serveraddr, serverlen);
    sendto(sockfd, filename, 10, 0, &serveraddr, serverlen); // sending filename information to server so it can know which file to delete
    recvfrom(sockfd, statusMessage, 1000, 0, &serveraddr, &serverlen);
    printf("%s\n", statusMessage);

    //simpler, just send file name, and recieve succcess status from server
  }

  //LS
  else if (strcmp(buf, "ls\n") == 0)
  {
    strcpy(commandBuffer, "ls"); //to send in a message for server to know action

    printf("Here are all of the files on the server:\n");
    serverlen = sizeof(serveraddr);

    sendto(sockfd, commandBuffer, 10, 0, &serveraddr, serverlen);

    recvfrom(sockfd, serverResponse, MAXFILESIZE, 0, &serveraddr, &serverlen);
    printf("%s\n", serverResponse);
  }

  //EXIT
  else if (strcmp(buf, "exit\n") == 0)
  {
    printf("Exiting program\n");
  }

  else
  {
    printf("\nPlease enter valid command. \n(NOTE: I broke up `put <filename>` into put, send command, then filename, so that there's less room for error with two separate commands and for it to be a more hierarchical/interactive program)\n");
    return 1;
  }

  /* print the server's reply */
}
