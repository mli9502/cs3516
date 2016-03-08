all: server client

server: server.o Ack.o Frame.o Packet.o
	g++ -o server server.o Ack.o Frame.o Packet.o

client: client.o Ack.o Frame.o Packet.o DataLinkLayer.o
	g++ -o client client.o Ack.o Frame.o Packet.o DataLinkLayer.o

Ack.o: Ack.cpp Ack.h
	g++ -c Ack.cpp

Frame.o: Frame.cpp Frame.h
	g++ -c Frame.cpp

Packet.o: Packet.cpp Packet.h
	g++ -c Packet.cpp

DataLinkLayer.o: DataLinkLayer.cpp DataLinkLayer.h
	g++ -c DataLinkLayer.cpp

clean:
	rm -f *.o server client