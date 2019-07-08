
CC = g++
INCLUDES =  -I/usr/local/src/HElib/src
CFLAGS = -O2 -std=c++11 -lm -pthread -DFHE_THREADS -DFHE_DCRT_THREADS -DFHE_BOOT_THREADS -lntl -lgmp -lboost_system
HELIBDIR = /usr/local/src/HElib/src/fhe.a


#g++ -I/home/ameki/github/HElib/src keygen.cpp /home/ameki/github/HElib/src/fhe.a -o keygen -lntl -lgmp -lm -pthread

keygen: keygen.cpp
	$(CC) $(INCLUDES) -o keygen keygen.cpp $(HELIBDIR) $(CFLAGS)

client: client.cpp
	$(CC) $(INCLUDES) -o client client.cpp $(HELIBDIR) $(CFLAGS)

server: server.cpp
	$(CC) $(INCLUDES) -o server server.cpp $(HELIBDIR) $(CFLAGS)

update: update.cpp
	$(CC) $(INCLUDES) -o update update.cpp $(HELIBDIR) $(CFLAGS)

data_enc: data_enc.cpp
	$(CC) $(INCLUDES) -o data_enc data_enc.cpp $(HELIBDIR) $(CFLAGS)

findm: findm.cpp
	$(CC) $(INCLUDES) -o findm findm.cpp $(HELIBDIR) $(CFLAGS)

client_datacollect: client_datacollect.cpp
	$(CC) $(INCLUDES) -o client_datacollect client_datacollect.cpp $(HELIBDIR) $(CFLAGS)

client_datacollect2_fixedinput: client_datacollect2_fixedinput.cpp
	$(CC) $(INCLUDES) -o client_datacollect2_fixedinput client_datacollect2_fixedinput.cpp $(HELIBDIR) $(CFLAGS)

client_datacollect2_generatedinput: client_datacollect2_generatedinput.cpp
	$(CC) $(INCLUDES) -o client_datacollect2_generatedinput client_datacollect2_generatedinput.cpp $(HELIBDIR) $(CFLAGS)
