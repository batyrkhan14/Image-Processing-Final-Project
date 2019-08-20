//#include <stdio.h> //printf
//#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
//#include <stdlib.h>
#include <unistd.h>
//#include <sys/types.h> 
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>

using namespace cv;
using namespace std;


void read(vector<char> &in, int &offset, short &val){
    val = (short)(((uchar)in[offset]))*256 + (short)(((uchar)in[offset+1]));
    offset += 2;
}

void vector_to_mat(Mat &frame, vector<char> &res){
    int offset = 0;
    short w, h;
    read(res, offset, w);
    read(res, offset, h);
    resize(frame, frame, Size(w, h));
    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
            Vec3b pixel;
            for (int k = 0; k < 3; k++){
                pixel[k] = (uchar)res[offset];
                offset++;
            }
            frame.at<Vec3b>(i, j) = pixel;
        }
    }
}

int main(int argc , char *argv[]){
    int sock, read_size;
    struct sockaddr_in server;
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1){
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
 
    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
    char buffer[3000000];
    namedWindow("Received Video", 1);
    vector<char> compressed;
    bool number = true;
    int sz = 0;
    int left = 0;
    while(  (number && (read_size = recv(sock, &buffer[0], 4, 0)) > 0) ||
            (!number && (read_size = recv(sock, &buffer[0], left, 0)) > 0)){
        if (number){
            number = false;
            sz = *((int*) buffer);
            left = sz;
            cout << "size: " << sz << endl;
            cout << "sz read size: " << read_size << endl;
            //write(sock, "accepted", 8);
        }
        else{
            compressed.insert(compressed.end(), buffer, buffer+read_size);
            cout << "read: " << read_size << endl;
            if (compressed.size() >= sz){
//                write(sock, "accepted", 8);            
                Mat frame(10, 10, CV_8UC3, Scalar(255));
                vector_to_mat(frame, compressed);
                imshow("Received Video", frame);
                waitKey(1); 
                compressed.clear();
                number = true;                  
            }
            left -= read_size;
            if (left < 0) left = 0;

        }
    }
     
    if (read_size == 0){
        puts("Client disconnected");
        fflush(stdout);
    }

    else if(read_size == -1){
        perror("recv failed");
    }

    close(sock);
    return 0;
}