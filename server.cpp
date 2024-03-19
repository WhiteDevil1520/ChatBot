#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <map>
#include <pthread.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_PENDING 5
#define BUFFER_SIZE 1024

using namespace std;

char *ip = "127.0.0.1";

// A struct to hold client information
struct ClientInfo {
    int sock_fd; // Socket file descriptor
    string username; // Client username
};

// A map to hold information of all connected clients
map<string, ClientInfo> client_map;

// Function to send a message to a specific client
void send_message(int receiver_sock_fd, const char* message) {
    if (write(receiver_sock_fd, message, strlen(message)) == -1) {
        perror("send");
    }
}

// Function to send a message to all connected clients except the sender
void send_broadcast_message(int sender_sock_fd, const char* message) {
    for (map<string, ClientInfo>::iterator  it = client_map.begin(); it != client_map.end(); it++) {
        if (it->second.sock_fd != sender_sock_fd) {
            send_message(it->second.sock_fd, message);
        }
    }
}

// Function to handle incoming connections
void handle_connection(void *arg) {//int client_sock_fd) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int client_sock_fd;

    client_sock_fd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());


    // Loop to receive and handle incoming messages from the client
    while ((bytes_received = recv(client_sock_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        string username;
        if (bytes_received == -1) {
            perror("recv");
            return;
        }

        if (bytes_received == 0) {
            // Connection closed by client
            break;
        }

        buffer[bytes_received] = '\0';
        string message(buffer);
        string input, param;
        istringstream iss(message);
        iss >> input >> param;

        // cout << input << endl;
        if (message.substr(0,6) == "login "){//input == "login"){ //message.substr(0,6) == "login ") {
            // username = param;
            username = message.substr(6);
            ClientInfo client = {client_sock_fd, username};
            client_map[username] = client;
            cout << username << " has joined the chat." << endl;
            string welcome = "Welcome " + username;
            send_message(client_sock_fd, welcome.c_str());
        }

        // Check if the received command is a logout command
        else if (message.substr(0,5) == "logout") {
            cout << "Successfully loggedout." << endl;
            client_map.erase(client_sock_fd);
        }

        // Check if the received command is a chat command
        else if (message.substr(0,5) == "chat ") {
            string sender;
            int target_sockid;
            for (map<string, ClientInfo>::iterator  it = client_map.begin(); it != client_map.end(); it++) {
                if (it->second.sock_fd == client_sock_fd) {
                    sender = it->second.username;   
                }
            }
            
            if(message.substr(0,6) == "chat @"){
                string share_message =  message.substr(6);
                int user_pos = share_message.find(" ");
                // cout << "user_pos" << user_pos << endl;
                string target_message = share_message.substr(user_pos, share_message.size());
                string target = message.substr(6, user_pos);

                for (map<string, ClientInfo>::iterator  it = client_map.begin(); it != client_map.end(); it++) {
                    if (it->second.username == target) {
                        target_sockid = it->second.sock_fd;   
                    }
                }

                string direct_msg = sender + ">> " + target_message.c_str();
                send_message(target_sockid, direct_msg.c_str());
            }
            else{
                string broadcast_msg = sender + ">> " + message.substr(5);
                send_broadcast_message(client_sock_fd, broadcast_msg.c_str());
            }
        }
    }

    // Close the socket when the client exits
    close(client_sock_fd);
    return(NULL);
}

int main(int argc, char *argv[]) {
  // Check if the port number is provided
  FILE *fPtr;
  int port;
   pthread_t tid;
    socklen_t sock_len;
    fd_set readfds, masterfds;
    int maxfd;
  if (argc < 2) {
  cout << "Please give configuration file." << endl;
  return 0;
  }
  // Create the socket
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
      cout << "Failed to create the server socket." << endl;
      return 0;
  }
  fPtr = fopen(argv[1], "r");
    if(fPtr == NULL) {
        perror("Unable to open file.\n");
        return EXIT_FAILURE;
    }

    if(fscanf(fPtr, "port: %d", &port) != 1) {
        perror("Error reading from configuration file.\n");
    }

    printf("Running on port %d\n", port);

  // Bind the socket to the IP address and port number
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;
  memset(&server_address, 0, sizeof(struct sockaddr_in));
    bzero((void*)&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);


  if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
      cout << "Failed to bind the server socket." << endl;
      return 0;
  }

  // Listen for incoming connections
  if (listen(server_socket, 10) == -1) {
      cout << "Failed to listen for incoming connections." << endl;
      return 0;
  }

    FD_ZERO(&masterfds);
    FD_SET(server_socket, &masterfds);
    maxfd = server_socket;
    cout << "server started on port = " << ntohs(server_address.sin_port) << endl;

    while(1){
		readfds = masterfds;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        for (; ;) {
            sock_len = sizeof(client_address);
            int client_address_length = sizeof(client_address);
            int client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_address_length); 
            //accept(serv_sockfd, (struct sockaddr *)&cli_addr, &sock_len);

            cout << "remote client IP == " << inet_ntoa(client_address.sin_addr);
            cout << ", port == " << ntohs(client_address.sin_port) << endl;

            int *sock_ptr;
            sock_ptr = (int *)malloc(sizeof(int));
            *sock_ptr = client_socket;

            pthread_create(&tid, NULL, &handle_connection, (void
                        *)sock_ptr);
        }
    }

  // Close the server socket
  close(server_socket);

  return 0;
}



