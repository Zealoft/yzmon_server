
source = src/main.cpp src/common/Logger.cpp src/Server.cpp
objects = tmp/sql.o tmp/actions.o tmp/Server.o tmp/Logger.o

build: $(source)
	g++ -c src/common/Logger.cpp -o tmp/Logger.o -std=gnu++11
	g++ -c src/sql.cpp -o tmp/sql.o $(shell mysql_config --cflags) $(shell mysql_config --libs) -std=gnu++11
	g++ -c src/actions.cpp -o tmp/actions.o $(shell mysql_config --cflags) $(shell mysql_config --libs) -std=gnu++11
	g++ -c src/Server.cpp -o tmp/Server.o $(shell mysql_config --cflags) $(shell mysql_config --libs) -std=gnu++11
	g++ $(objects) src/main.cpp $(shell mysql_config --cflags) $(shell mysql_config --libs) -o yzmond -std=gnu++11
	
.PHONY : clean
clean: 
	- rm -rf $(objects) yzmond