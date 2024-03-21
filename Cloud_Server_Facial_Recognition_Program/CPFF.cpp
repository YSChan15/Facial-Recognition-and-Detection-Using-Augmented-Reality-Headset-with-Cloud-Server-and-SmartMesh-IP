/*
* Author: Brycen Hillukka (CE/EE)
* Group Members: 
*   Yu Sheng Chan (EE)
*   Brandon Wieberdink (EE)
*   Hayden Scott (CE)
* Version: 1.0.0
* Course: ECE 461 and ECE 462
* Course Name: Senior Design Project
* Project Name: Facial Recognition and Detection using Augmented Reality Headset with Cloud Server and SmartMesh IP Network
*
* This program runs on the cloud server which creates TCP IPV4 socket
* connections on ports 3419 and 3420 to the Microsoft HoloLens 2 AR
* headset. The cloud server waits to receive and process an image and
* sensor data sent from the Microsoft HoloLens 2 AR headset. Once both
* are received, the server program runs facial recognition on the image
* using a local database. The name of the recognized person is sent to
* the SmartMesh IP network along with the sensor data, which distributes
* that information to its motes.
*/


// Used for TCP IP Sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>

// Included from previous C program
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <time.h>

// Included for C++ version of program
#include <iostream>
#include <string>
#include <cstring>

// Included for OpenCV Fisherface
#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <fstream>
#include <sstream>

#define SIZE 1024
#define ARPORT 3419
#define SMARTMESHPORT 3420

using namespace cv;
using namespace cv::face;
using namespace std;

// Write received data from a socket to a file
void write_file(int sockfd){
  int n;
  FILE *fp;
  char filename[] = "recv.txt";
  char buffer[SIZE];
 
  fp = fopen(filename, "wb");
  while (1) {
    n = recv(sockfd, buffer, SIZE, 0);
    cout << "Receiving" << endl;
    if (n <= 0){
      break;
      return;
    }
    send(sockfd, "OK",2,0);
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
  }
  fclose(fp);
  cout << "leaving" << endl;
  return;
}

// Write the received data to an image file
int receive_image(int socket)
{ // Start function 

    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

    char imagearray[10241],verify = '1';
    FILE *image;

    //Find the size of the image
    do{
        stat = read(socket, &size, sizeof(int));
    }while(stat<0);

    cout << "Packet received." << endl;
    cout << "Packet size: " << stat << endl;
    cout << "Image size: " << size << endl << endl;

    char buffer[] = "OK";

    //Send our verification signal
    do{
        stat = send(socket, "OK\n", 4, 0);
    }while(stat<0);

    cout << "Reply sent" << endl << endl;

    image = fopen("Received_Image.jpg", "w");

    if( image == NULL) {
        cout << "Error has occurred. Image file could not be opened." << endl;
        return -1; }

    //Loop while we have not received the entire file yet

    int need_exit = 0;

    int buffer_fd, buffer_out;

    while(recv_size < size) {
       
        do{
            read_size = read(socket,imagearray, 10241);
        }while(read_size <0);

        cout << "Packet number received: " << packet_index << endl;
        cout << "Packet size: " << read_size << endl;


        //Write the currently read data into our image file
        write_size = fwrite(imagearray,1,read_size, image);
        cout << "Written image size: " << write_size << endl;

        if(read_size !=write_size) {
            cout << "error in read write" << endl;    }


        //Increment the total number of bytes read
        recv_size += read_size;
        packet_index++;
        cout << "Total received image size: " << recv_size << endl << endl << endl;
            
        send(socket, "OK\n", 4, 0); // Send acknowledgement signal
    }
    
    usleep(50000);
    send(socket, "Done\n", 6, 0); // Send acknowledgement signal
    
    fclose(image);
    
    return 0;
}

static Mat norm_0_255(InputArray _src) {
    Mat src = _src.getMat();
    // Create and return normalized image:
    Mat dst;
    switch (src.channels()) {
    case 1:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
        break;
    case 3:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
        break;
    default:
        src.copyTo(dst);
        break;
    }
    return dst;
}
static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(Error::StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if (!path.empty() && !classlabel.empty()) {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }

    for (int i = 0; i < images.size(); i++)
        resize(images[i], images[i], Size(200, 200));
}

int main ()
{
    cout << "Welcome to the AWS cloud server facial recognition program." << endl;
    
    int err;                // Used for checking errors from system calls

    int sSocket;
    int ARSocket;           // The AR headset socket
    int SmartMeshSocket;    // The SmartMesh socket
    
    struct sockaddr_in AR_addr;         // The AR headset address
    struct sockaddr_in SmartMesh_addr;  // The SmartMesh address
    
    socklen_t ARSocLen;           // The length of the AR headset socket
    socklen_t SmartMeshSocLen;    // The length of the SmartMesh socket
    
    char Buf[100];          // The buffer for socket communication
    char input_str[50];     // The input string
    char OKmessage[] = "OK\n";
    
    // CONNECT TO THE AR HEADSET
    
    // Create a socket
    sSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (sSocket == -1)
    {
        perror("AR socket creation failed.");
        exit(1);
    }
    
    memset(&AR_addr, 0, sizeof(struct sockaddr_in));
    AR_addr.sin_family = AF_INET;                // IPV4
    AR_addr.sin_port = htons(ARPORT);        // Select server port
    AR_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Selects any available address
    err = bind(sSocket, (struct sockaddr*) &AR_addr, sizeof(struct sockaddr_in)); // Bind the socket
    if (err == -1)
    {
        perror("AR socket bind address to socket failed.");
        exit(2);
    }
    
    printf("Starting to listen for AR headset on port 3419.\n");
    
    // Listens for incoming connections
    err = listen(sSocket, 5);
    if (err == -1)
    {
        perror("AR socket listen failed.");
        exit(3);
    }
    
    // Accepts any incoming connections
    ARSocket = accept(sSocket, (struct sockaddr *) &AR_addr, &ARSocLen);
    if (ARSocket == -1)
    {
        perror("AR socket accept failed.");
        exit(4);
    }
    
    cout << "Connected to AR headset." << endl;
    
    // CONNECT TO THE SMARTMESH IP NETWORK
    /*
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
    
    cout << "Starting to listen for SmartMesh IP network on port 3420." << endl;
    
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
    
    cout << "Connected to SmartMesh." << endl;
    */
    bzero(Buf, 100);    // clear the buffer
    
    // START FISHERFACE PROGRAM
    
    string database[4] = {"Brycen Hillukka\n", "Yu Sheng Chan\n", "Brandon Wieberdink\n", "Hayden Scott\n"};
    
    string output_folder = ".";
    
    string fn_csv = "FaceDataX.txt"; // Get the path to your CSV
    
    // These vectors hold the images and corresponding labels.
    vector<Mat> images;
    vector<int> labels;
    
    // Read in the data. This can fail if no valid
    // input filename is given.
    try {
        read_csv(fn_csv, images, labels);
    }
    catch (const cv::Exception& e) {
        cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }
    // Quit if there are not enough images for this demo.
    if (images.size() <= 1) {
        string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
        CV_Error(Error::StsError, error_message);
    }
    
    // Get the height from the first image
    int height = images[0].rows;
    
    Ptr<FisherFaceRecognizer> model = FisherFaceRecognizer::create();
    
    try{
        model->read("savedAI_FF.txt");   // Use to read previously trained AI model
        cout << "Reloaded previously trained AI model." << endl;
    }
    catch (const exception& e){
        model->train(images, labels);
        model->write("savedAI_FF.txt");  // Use to save trained AI model
        cout << "Saving trained AI model." << endl;
    }
    
    send(ARSocket, "AllConnectionsMade\n", 20, 0);  // Tell AR headset that all connections were made
    
    Mat receivedImage;
    int numImages = 0;
    int numImagesReceived = 0;
    
    string allReceivedNames = "";
    
    while (1) 
    {
        read(ARSocket, Buf, 100);
        
        if (strcmp(Buf, "Image") == 0)
        {
            printf("%s\n", Buf);
            bzero(Buf, 100);
            send(ARSocket, OKmessage,4,0); // Send acknowledgement signal
            
            receive_image(ARSocket);
            
            receivedImage = imread("Received_Image.jpg", 0);  // Read in the image
            resize(receivedImage, receivedImage, Size(200, 200));   // Resize the image
            
            cout << "Received image from the AR headset." << endl;
            
            numImagesReceived++;
        }
        else if (strcmp(Buf, "SensorData") == 0)
        {
            cout << Buf << endl;
            bzero(Buf, 100);
            send(ARSocket, OKmessage,4,0); // Send acknowledgement signal
            
            read(ARSocket, Buf, 100);
            cout << Buf << endl;
            send(ARSocket, OKmessage,4,0); // Send acknowledgement signal
            
            Buf[strlen(Buf)] = '\n';
            
            // Predict the received face
            int predictedLabel = model->predict(receivedImage); // Predict the received face
    
            if (predictedLabel >= 0 && predictedLabel < 4)
            {
                cout <<  "Recognized: " << database[predictedLabel];
                
                usleep(500000); // Delay for AR headset to receive name after OK signal
                
                // Send name to AR headset
                send(ARSocket, database[predictedLabel].c_str(), database[predictedLabel].size() + 1, MSG_CONFIRM);
                
                if (numImagesReceived == numImages)
                {
                    numImagesReceived = 0;
                    numImages = 0;
                    
                    allReceivedNames += database[predictedLabel];   // Append the last name to the string
                    cout << allReceivedNames << endl;
                    
                    // Send name to SmartMesh IP network
                    err = send(SmartMeshSocket, allReceivedNames.c_str(), allReceivedNames.size(), MSG_CONFIRM);
                    
                    if (err == -1)
                    {
                        perror("SmartMesh socket send failed.");
                        exit(5);
                    }
                    
                    // Send the sensor data to the SmartMesh IP network
                    err = send(SmartMeshSocket, Buf, strlen(Buf), 0);
                
                    if (err == -1)
                    {
                        perror("SmartMesh socket send failed.");
                        exit(5);
                    }
            
                    cout << "Sent message to the SmartMesh IP network." << endl;
                    
                    allReceivedNames = "";  // Clear the received names
                }
                else
                {
                    int newlinePos = database[predictedLabel].find("\n");
                    allReceivedNames += database[predictedLabel].substr(0, newlinePos) + ",";   // Append each name to a string for the SmartMesh
                }
            }
            else    // No face was recognized in the image
            {
                send(ARSocket, "Not recognized\n", 16, MSG_CONFIRM);
                
                send(SmartMeshSocket, "Not recognized\n", 16, MSG_CONFIRM);
            }
            
            receivedImage.release();
            read(ARSocket, Buf, 100);
            
            usleep(50000);
            
            send(ARSocket, "Complete\n", 10, MSG_CONFIRM);
            
            bzero(Buf, 100);
        }
        else if (strcmp(Buf, "Disconnect") == 0)
        {
            cout << Buf << endl;
            bzero(Buf, 100);
            send(ARSocket, OKmessage,4,0); // Send acknowledgement signal
            
            sleep(1);
            
            send(ARSocket, OKmessage,4,0); // Send acknowledgement signal
            
            err = close(ARSocket);
            if (err == -1)
            {
                perror("AR headset disconnect failed.");
                exit(5);
            }
            else
            { 
                cout << "AR headset disconnected successfully." << endl;
                    
                cout << "Waiting for the AR headset to reconnect." << endl;
                    
                cout << "Starting to listen for AR headset on port 3419." << endl;
                    
                // Listens for incoming connections
                err = listen(sSocket, 5);
                if (err == -1)
                {
                    perror("AR socket listen failed.");
                    exit(3);
                }
                
                // Accepts any incoming connections
                ARSocket = accept(sSocket, (struct sockaddr *) &AR_addr, &ARSocLen);
                if (ARSocket == -1)
                {
                    perror("AR socket accept failed.");
                    exit(4);
                }
                
                send(ARSocket, "AllConnectionsMade\n", 20, 0);  // Tell AR headset that all connections were made
                    
                cout << "Reconnected to AR headset." << endl;
            }
        }
        else if (strcmp(Buf, "NoFaces") == 0)
        {
            cout << Buf << endl;
            bzero(Buf, 100);
            send(ARSocket, "Complete\n", 10, MSG_CONFIRM);
            
            send(SmartMeshSocket, "No faces detected\n", 19, MSG_CONFIRM);
        }
        else
        {
            cout << Buf << endl;
            numImages = atoi(Buf);
            bzero(Buf, 100);
            send(ARSocket, OKmessage,4,0); // Send acknowledgement signal
        }
    }
    
    return 0;
}
