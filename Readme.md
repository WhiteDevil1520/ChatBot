File description
	1. server.cpp: server program
	2. client.cpp: client program
        
Compiling
      1. "make server": creating an executable for server.cpp file named server.x
      2. "make client": creating an executable for client.cpp file named client.x
      
Executing
	1. server execution: server.x server_config.txt
	2. client execution: client.x client_config.txt
	where serverconfig.txt and clientconfig.txt are configuration files

	
About pthread and select():

1. Here I have used pthread in server programto create a new thread to handle each client's connection.
The function process_connection() is called in a new thread for each client connection. The arg parameter of the process_connection() is a pointer to an int that contains the client socket file descriptor. The function then reads and processes messages from the client using the socket file descriptor. To create a new thread for each client connection, the program calls pthread_create and passes a pointer to the process_connection() and the client socket file descriptor as arguments.

2. Here I have used select() is used to allow the program to wait for input from the server socket and standard input simultaneously. When input is detected on the server socket, the program reads the input and displays it on the screen. When input is detected on standard input, the program reads the input and sends it to the server.

NOTE:
Sometimes the socket binding is acting wierdly because of the port unavailability. With the change in port number everything is working normally.