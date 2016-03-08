#author: Mengwen Li (mli2)
all: mli2_MEClient mli2_MEServer

mli2_MEClient: mli2_MEClient.o helper.o Person.o
	g++ -o mli2_MEClient mli2_MEClient.o helper.o Person.o

mli2_MEServer: mli2_MEServer.o Person.o PersonDb.o helper.o
	g++ -o mli2_MEServer mli2_MEServer.o Person.o PersonDb.o helper.o

mli2_MEClient.o: mli2_MEClient.cpp helper.h
	g++ -c mli2_MEClient.cpp

mli2_MEServer.o: mli2_MEServer.cpp Person.h PersonDb.h helper.h
	g++ -c mli2_MEServer.cpp
	
Person.o: Person.cpp Person.h
	g++ -c Person.cpp

PersonDb.o: PersonDb.cpp PersonDb.h Person.h helper.h
	g++ -c PersonDb.cpp

helper.o: helper.cpp helper.h
	g++ -c helper.cpp

clean:
	rm -f mli2_MEServer mli2_MEClient *.o 


