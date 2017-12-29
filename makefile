make: 
	g++ -pthread -std=c++11 -o Server.out Game.cpp Server.cpp Utils.cpp Timer.cpp IdleChecker.cpp
