#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#define SIZE 1024
#define ARPORT 
#define SMARTMESHPORT 
 
void write_file(int sockfd){
  int n;
  FILE *fp;
  char *filename = "recv.txt";
  char buffer[SIZE];
 
  fp = fopen(filename, "wb");
  while (1) {
    n = recv(sockfd, buffer, SIZE, 0);
    printf("Receiving\n");
    if (n <= 0){
      break;
      return;
    }
    send(sockfd, "OK",2,0);
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
  }
  fclose(fp);
  printf("leaving\n");
  return;
}

int receive_image(int socket)
{ // Start function 

    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

    char imagearray[10241],verify = '1';
    FILE *image;

    //Find the size of the image
    do{
        stat = read(socket, &size, sizeof(int));
    }while(stat<0);

    printf("Packet received.\n");
    printf("Packet size: %i\n",stat);
    printf("Image size: %i\n",size);
    printf(" \n");

    char buffer[] = "OK";

    //Send our verification signal
    do{
        stat = write(socket, &buffer, 3);
    }while(stat<0);

    printf("Reply sent\n");
    printf(" \n");

    image = fopen("capture2.jpg", "w");

    if( image == NULL) {
        printf("Error has occurred. Image file could not be opened\n");
    return -1; }

    //Loop while we have not received the entire file yet


    int need_exit = 0;
    //struct timeval timeout = {10,0};

    int buffer_fd, buffer_out;

    while(recv_size < size) {
       
        do{
            read_size = read(socket,imagearray, 10241);
        }while(read_size <0);

        printf("Packet number received: %i\n",packet_index);
        printf("Packet size: %i\n",read_size);


        //Write the currently read data into our image file
        write_size = fwrite(imagearray,1,read_size, image);
        printf("Written image size: %i\n",write_size); 

        if(read_size !=write_size) {
            printf("error in read write\n");    }


        //Increment the total number of bytes read
        recv_size += read_size;
        packet_index++;
        printf("Total received image size: %i\n",recv_size);
        printf(" \n");
        printf(" \n");
            
        send(socket, "OK", 3, 0); // Send acknowledgement signal
    }
    
    send(socket, "Done", 5, 0); // Send acknowledgement signal
}

int main ()
{
    printf("Welcome to the AWS cloud server facial recognition program.\n");
    
    int err;                // Used for checking errors from system calls

    int ARSocket;           // The AR headset socket
    int SmartMeshSocket;    // The SmartMesh socket
    
    struct sockaddr_in AR_addr;         // The AR headset address
    struct sockaddr_in SmartMesh_addr;  // The SmartMesh address
    
    int ARSocLen;           // The length of the AR headset socket
    int SmartMeshSocLen;    // The length of the SmartMesh socket
    
    char Buf[100];          // The buffer for socket communication
    char input_str[50];     // The input string
    char OKmessage[] = "OK";
    
    // CONNECT TO THE AR HEADSET
    
    // Create a socket
    ARSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ARSocket == -1)
    {
        perror("AR socket creation failed.");
        exit(1);
    }
    
    memset(&AR_addr, 0, sizeof(struct sockaddr_in));
    AR_addr.sin_family = AF_INET;                // IPV4
    AR_addr.sin_port = htons(ARPORT);        // Select server port
    AR_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Selects any available address
    err = bind(ARSocket, (struct sockaddr*) &AR_addr, sizeof(struct sockaddr_in)); // Bind the socket
    if (err == -1)
    {
        perror("AR socket bind address to socket failed.");
        exit(2);
    }
    
    printf("Starting to listen for AR headset on port 3419.\n");
    
    // Listens for incoming connections
    err = listen(ARSocket, 5);
    if (err == -1)
    {
        perror("AR socket listen failed.");
        exit(3);
    }
    
    // Accepts any incoming connections
    ARSocket = accept(ARSocket, (struct sockaddr *) &AR_addr, &ARSocLen);
    if (ARSocket == -1)
    {
        perror("AR socket accept failed.");
        exit(4);
    }
    
    printf("Connected to AR headset.\n");
    
    printf("Ready to receive image from the AR headset.\n");
    
    // CONNECT TO THE SMARTMESH IP NETWORK
    
    // Create a socket
    SmartMeshSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (SmartMeshSocket == -1)
    {
        perror("SmartMesh socket creation failed.");
        exit(1);
    }
    
    memset(&SmartMesh_addr, 0, sizeof(struct sockaddr_in));
    SmartMesh_addr.sin_family = AF_INET;                // IPV4
    SmartMesh_addr.sin_port = htons(SMARTMESHPORT);        // Select server port
    SmartMesh_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Selects any available address
    err = bind(SmartMeshSocket, (struct sockaddr*) &SmartMesh_addr, sizeof(struct sockaddr_in)); // Bind the socket
    if (err == -1)
    {
        perror("SmartMesh socket bind address to socket failed.");
        exit(2);
    }
    
    printf("Starting to listen for SmartMesh IP network on port 3420.\n");
    
    // Listens for incoming connections
    err = listen(SmartMeshSocket, 5);
    if (err == -1)
    {
        perror("SmartMesh socket listen failed.");
        exit(3);
    }
    
    // Accepts any incoming connections
    SmartMeshSocket = accept(SmartMeshSocket, (struct sockaddr *) &SmartMesh_addr, &SmartMeshSocLen);
    if (SmartMeshSocket == -1)
    {
        perror("SmartMesh socket accept failed.");
        exit(4);
    }
    
    printf("Connected to SmartMesh.\n");
    
    //sprintf(Buf, "Message from AWS Cloud9");
    
    bzero(Buf, 100);
    
    while (1) {
    read(ARSocket, Buf, 100);
    
    if (strcmp(Buf, "Image") == 0)
    {
        printf("%s\n", Buf);
        bzero(Buf, 100);
        send(ARSocket, OKmessage,3,0); // Send acknowledgement signal
        
        receive_image(ARSocket);
        
        printf("Received image from the AR headset.\n");
    }
    else if (strcmp(Buf, "SensorData") == 0)
    {
        printf("%s\n", Buf);
        bzero(Buf, 100);
        send(ARSocket, OKmessage,3,0); // Send acknowledgement signal
        
        read(ARSocket, Buf, 100);
        printf("%s\n", Buf);
        send(ARSocket, OKmessage,3,0); // Send acknowledgement signal
        
        Buf[strlen(Buf)] = '\n';
        
        err = send(SmartMeshSocket, Buf, strlen(Buf)+1, 0);
        
        if (err == -1)
        {
            perror("SmartMesh socket send failed.");
            exit(5);
        }
    
        printf("Sent message to the SmartMesh IP network.\n");
        
        bzero(Buf, 100);
    }
    
    }
    
    return 0;
}
