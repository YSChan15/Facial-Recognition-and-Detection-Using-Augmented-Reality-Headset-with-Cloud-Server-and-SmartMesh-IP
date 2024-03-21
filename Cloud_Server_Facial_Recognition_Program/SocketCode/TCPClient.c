#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <strings.h>
#include <string.h>

#define SERVERPORT 
#define SERVERPORTSTR ""
#define SERVERIP ""
#define SIZE 1024

void send_file(FILE *fp, int sockfd){
  int n;
  char data[SIZE] = {0};
 
  while(fgets(data, SIZE, fp) != NULL) {
  	printf("Sending\n");
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    recv(sockfd, data, sizeof(data), 0);
    bzero(data, SIZE);
  }
  printf("leaving\n");
}

int send_image(int socket){

   FILE *picture;
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[256];
   packet_index = 1;

   picture = fopen("Selfie.jpg", "r");
   printf("Getting Picture Size\n");   

   if(picture == NULL) {
        printf("Error Opening Image File"); } 

   fseek(picture, 0, SEEK_END);
   size = ftell(picture);
   fseek(picture, 0, SEEK_SET);
   printf("Total Picture size: %i\n",size);

   //Send Picture Size
   printf("Sending Picture Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send Picture as Byte Array
   printf("Sending Picture as Byte Array\n");

   do { //Read while we get errors that are due to signals.
      stat=read(socket, &read_buffer , 255);
      printf("Bytes read: %i\n",stat);
   } while (stat < 0);

   printf("Received data in socket\n");
   printf("Socket data: %c\n", read_buffer);

   while(!feof(picture)) {
   //while(packet_index = 1){
      //Read from the file into our send buffer
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

      //Send data through our socket 
      do{
        stat = write(socket, send_buffer, read_size);  
      }while (stat < 0);

      printf("Packet Number: %i\n",packet_index);
      printf("Packet Size Sent: %i\n",read_size);     
      printf(" \n");
      printf(" \n");


      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
     }
}


int main ()
{
	int err;
	int cSocket;
	int cSize;
	struct sockaddr_in cAddr;
	int cSocLen;
	char Buf[100];
	
	FILE *fp;
	char *filename = "test.txt";
		
	memset(&cAddr, 0, sizeof(struct sockaddr_in));
	cAddr.sin_family = AF_INET;
	cAddr.sin_port = htons(SERVERPORT);
	cAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	bzero(&(cAddr.sin_zero), 8);
	cSize = sizeof(struct sockaddr);

	cSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (cSocket == -1)
	{
		perror("socket creation failed");
		exit(1);
	}
	
	printf("Ready to connect\n");
	err = connect(cSocket, (struct sockaddr *)&cAddr, sizeof(struct sockaddr_in));
	if (err == -1)
	{
		perror("connect failed");
		exit(2);
	}
	
	printf("Connected\n");
	/*err = recv(cSocket, Buf, 100, 0);
	if (err == -1)
	{
		perror("read failed");
		exit(3);
	}*/
	/*
	fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror("[-]Error in reading file.");
    exit(1);
  }
 
  send_file(fp, cSocket);*/
  send_image(cSocket);
	
	printf("%s\n", Buf);
}
