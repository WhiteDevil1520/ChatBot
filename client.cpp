#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string>
#include <pthread.h>
#include <fstream>
#include <fcntl.h>
#include <string.h>
#include <cstring>

// #define LENGTH 2048;

using namespace std;
const unsigned MAXBUFFER = 1024;

int socket_id;

// void *connect_with_server() {
// 	int socket_info;
// 	char buffer[MAXBUFFER];
// 	// pthread_detach(pthread_self());
// 	while (1) {
// 		socket_info = read(socket_id, buffer, MAXBUFFER);
// 		if (socket_info <= 0) {
// 			if (socket_info == 0) {
// 				printf("Server connection closed.\n");
// 			} else {
// 				printf("Unexpected halt!!!\n");
// 			}
// 			close(socket_id);
// 			exit(1);
// 		}
// 		buffer[socket_info] = '\0';
// 		// cout << buffer << endl;
// 	}
// }

int main(int argc, char **argv) {
	int rv, flag;
	struct addrinfo hints, *res, *ressave;
	pthread_t tid;
	string client_config;
	string host;
	int port = 0;
	int socket_id;

	if (argc != 2) {
      cout << "Usage: client.x client_config.txt." << endl;
      exit(1);
  }

  ifstream clientConfigFile(argv[1]);
	if(clientConfigFile) {
		string input; 
		while(getline(clientConfigFile,input)) {
			if(input.substr(0, 6) == "host: ") {
				host = input.substr(6);
				printf("%s\n", host.c_str());
			}
			if(input.substr(0,6) == "port: ") {
				port = stoi(input.substr(6));
				printf("%d\n", port);
			}
		}	   
	}
	else {
		cerr << "Unable to open file." << endl;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	socket_id = socket(AF_INET, SOCK_STREAM, 0);
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(host.c_str());

  // Connect to Server
  int err = connect(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// pthread_create(&tid, NULL, &connect_with_server, NULL);

	string input;
	bool loggedInUser = false;
	int n;
	char buf[MAXBUFFER];

	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

	/* set up select */
	fd_set read_fds;
	fd_set master_fds;
	int fdmax = socket_id > STDIN_FILENO ? socket_id : STDIN_FILENO;
	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);
	FD_SET(socket_id, &master_fds);
	FD_SET(STDIN_FILENO, &master_fds);

	// fgets(message, LENGTH, stdin);
	//getline(cin, input)
	while (1) {
		read_fds = master_fds;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
				perror("select");
				exit(1);
		}
		if (FD_ISSET(socket_id, &read_fds)) {
            n = read(socket_id, buf, MAXBUFFER);
            if (n <= 0) {
                if (n == 0) {
                    cout << "Server connection closed." << endl;
                } else {
                    cout << "unexpected halt!!" << endl;
                }
                close(socket_id);
                exit(1);
            }
            buf[n] = '\0';
            cout << buf << endl;
    }
		if (FD_ISSET(STDIN_FILENO, &read_fds)) {
			getline(cin, input);
			if(input.substr(0,5) == "login") {
				if(loggedInUser){
					cout << "You are logged in. Use chat to share messages." << endl;
					continue;
				}
				else{
					loggedInUser = true;
				}
			}
			if(input.substr(0,4) ==  "exit") {
				if(loggedInUser){
					cout << "Use logout before exiting." << endl;
				}
				else{
					cout << "Exited successfully." << endl;
					close(socket_id);
					exit(0);
				}
			}
			if(input.substr(0,6) == "logout") {
				loggedInUser=false;
			}	
			write(socket_id, input.c_str(), strlen(input.c_str())+1);
		}
	}
}
