Inside the tar file, there will be two folders, client_files and server_files, this readme, and a Makefile. In order to compile the needed files, 'make all' can be executed in bash in the directory, and the executables will be placed in the respective folders. 

The server program can be run with the following layout:

./server <port num>

...and the client similarly, but also specifying the location of the server:

./client <server_dest> <server_port_num>

When running the program locally, the server_dest can just be localhost, and the port number to provide for the server and the port number for the client to connect can be the same. 

Run the server first, so that the client can have a server to connect to.


HOW TO USE PROGRAM:
When running the client, the user will be asked to enter a command first: either get, put, delete, ls, or exit. Then, if the command requires a filename, the user will be prompted with which file they would like to get, put, or delete from the server. The client will be given appropiate messages for whether there request was met or if it wasn't, and the server will have some logging as well. 

