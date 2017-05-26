#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>
#include <cmath>
using namespace cv;
using namespace std;
 
void write(vector<char> &v, short x){
    v.push_back((char)(x/256));
    v.push_back((char)(x%256));
}

void mat_to_vector(Mat &frame, vector<char> &res){
    short w = frame.cols;
    short h = frame.rows;
    write(res, w);
    write(res, h);
    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
            Vec3b pixel = frame.at<Vec3b>(i, j);
            for (int k = 0; k < 3; k++){
                res.push_back((char)pixel[k]);
            }
        }
    }
}

int main(int argc, char *argv[]){
    int socket_desc, client_sock, c, read_size;
    struct sockaddr_in server, client;
    char client_message[2000];
     
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1){
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));
     
    if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("bind failed. Error");
        return 1;
    }

    puts("bind done");
     
    listen(socket_desc , 3);
     
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0){
        perror("accept failed");
        return 1;
    }

    puts("Connection accepted");

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        throw "Error when reading steam_avi";
    }
    Mat frame;
    for ( ; ; ){
        cap >> frame;
        if (frame.empty())
            break;
        vector<char> buffer;
        mat_to_vector(frame, buffer);
        int sz = buffer.size();
        cout << sz << endl;
        if(send(client_sock, &sz, sizeof(int), 0) < 0){
            puts("Send failed");
            return 1;
        }        
        char client_reply[2000];
        // if(recv(client_sock, client_reply, 2000, 0) < 0){
        //     puts("recv failed");
        //     break;
        // }        
        if(send(client_sock, &buffer[0], buffer.size(), 0) < 0){
            puts("Send failed");
            return 1;
        }
        // if(recv(client_sock, client_reply, 2000, 0) < 0){
        //     puts("recv failed");
        //     break;
        // }
        //puts(client_reply);
    }

     
    return 0;
}