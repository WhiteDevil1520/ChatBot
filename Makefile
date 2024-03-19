server: server.cpp
	gcc -pthread -fpermissive -lstdc++ -std=c++11 -o server.x server.cpp

client: client.cpp
	gcc -pthread -fpermissive -lstdc++ -std=c++11 -o client.x client.cpp

clean:
	rm -rf *.out *.o *.x