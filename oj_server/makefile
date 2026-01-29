oj_server:oj_server.cc
	g++ -o $@ $^ -g -I./include -L./lib -std=c++11 -lpthread -lctemplate -ljsoncpp -lmysqlclient -ldl -lssl -lcrypto
.PHONY:clean
clean:
	rm -f oj_server