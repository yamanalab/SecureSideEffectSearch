
CC = g++
INCLUDES =  -I/home/ameki/github/HElib/src
CFLAGS = -O2 -std=c++11 -lm -pthread -lntl -lgmp -lboost_system
HELIBDIR = /home/ameki/github/HElib/src/fhe.a


#g++ -I/home/ameki/github/HElib/src keygen.cpp /home/ameki/github/HElib/src/fhe.a -o keygen -lntl -lgmp -lm -pthread

keygen: keygen.cpp
	$(CC) $(INCLUDES) -o keygen keygen.cpp $(HELIBDIR) $(CFLAGS)

client: client.cpp
	$(CC) $(INCLUDES) -o client client.cpp $(HELIBDIR) $(CFLAGS)

server: server.cpp
	$(CC) $(INCLUDES) -o server server.cpp $(HELIBDIR) $(CFLAGS)

update: update.cpp
	$(CC) $(INCLUDES) -o update update.cpp $(HELIBDIR) $(CFLAGS)
