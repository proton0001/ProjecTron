all: server.out main_multiple_players.out

server.o: server.cpp
	g++ -c -g -std=c++11 server.cpp
	
server.out: server.o
	g++ server.o -o server.out -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lpthread

main_multiple_players.o: main_multiple_players.cpp
	g++ -c -g -std=c++11 main_multiple_players.cpp

main_multiple_players.out: main_multiple_players.o
	g++ main_multiple_players.o -o main_multiple_players.out -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lpthread

