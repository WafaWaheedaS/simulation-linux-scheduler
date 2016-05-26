#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>

#define CD "cd"       	    // CHANGE DIRECOTRY
#define PUT_FILE "putf"	    // GET FILE FROM SERVER
#define GET_FILE "getf"     // SAVE FILE TO SERVER
#define TERMINATE "eof"     //END OF FILE

char ** tokenizer(char command[]) {

char *buff, *argv[2];
int i = 0;

buff = strtok(command, " ");

while (buff != NULL && buff != "null") {
argv[i] = buff;
buff = strtok(NULL, " ");
i++;
}

if (i == 2)
return argv;

else
return NULL;

}

void saveFile(char* filename, int clientSock) {

//File *fd ;

//= *(int*) sock_desc;
FILE *fp; //file pointer

char buffer[4096];
char server_reply[4096];

fp = fopen("OutPutedFileName.txt", "ab+");

if (fp == NULL) {
printf("Could Not Create File!\n");

}

else {
printf("Starting to Recieve Your File....<%s>\n", filename);

}

/* Time to Receive the File */

int noOfLines;
int writtenLines;
while (1) {

noOfLines = read(clientSock, buffer, strlen(buffer));

if (noOfLines < 0) {
error("ERROR reading from socket");
break;
} else if (noOfLines == 0) {
printf("Got Your File %s \n", filename);
break;
} else {

noOfLines = fwrite(buffer, sizeof(buffer), 1, fp);

if (noOfLines < 0)
error("ERROR writing in file");

writtenLines = fprintf(fp, buffer, sizeof(buffer));

if (writtenLines < 0)
error("ERROR writing to file Locally");

bzero(buffer, 4096);
}

} //end child while loop
fclose(fp);
}

int main(int argc, char *argv[]) {
int sock;
struct sockaddr_in server;
char command[1000], server_reply[2000];

//Create socket
sock = socket(AF_INET, SOCK_STREAM, 0);
if (sock == -1) {
printf("cound'n create socket");
}
puts("Socket created");

server.sin_addr.s_addr = inet_addr("127.0.0.1");
server.sin_family = AF_INET;
server.sin_port = htons(8882);

//Connect to remote server
if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
printf("connection error");
return 1;
}

puts("Connected\n");

//keep communicating with server

while (1) {
printf("\tHere are possible commands you can use \n");
printf("\tTO send a file write <send> <filename.extention>\n");
printf(
"\tTO get a file from server write <get> <filename.extention>\n");
printf("\tTO change direcotory on the server side write <cd> <path>\n");
printf("\tEnter Your Comman here : ");
gets(command);

//Send some data
//char* token = strtok(command, " ");
//int i=1;

char **argv = tokenizer(command);
if (argv == NULL) {
printf(
"Please Enter a correct command with the mentioned criteria!\n");
continue;
}

if (strcmp(CD, argv[0]) == 0) {

printf("Changing Directory Reuqest Sent to Server\n");
}

else if (strcmp(PUT_FILE, argv[0]) == 0) {

printf("not implemented : Saving your file please wait....");

}

else if (strcmp(GET_FILE, argv[0]) == 0) {

int noOfL = write(sock, command, strlen(command));
if (noOfL < 0) {
printf("Could write to socket\n");
return 1;
}

saveFile(argv[1], sock);

}

else if (strcmp(TERMINATE, argv[0]) == 0) {

printf("Closing Connection....");
puts(server_reply);
break;

}

else {
printf(" Sorry this is unrecogrnized Command\n");

}

puts("Server reply :");
puts(server_reply);
}

close(sock);
return 0;
}