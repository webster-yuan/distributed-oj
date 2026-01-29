.PHONY: all
all: testMD5 testGPTAPI testCtemplate

testMD5: testMD5.cc
	g++ -o $@ $^ -I ../oj_server/include -L ../oj_server/lib -std=c++11 -lpthread -lctemplate -ljsoncpp -lmysqlclient -ldl -lssl -lcrypto

testGPTAPI: testGPTAPI.cc
	g++ -o $@ $^ -lcurl -ljsoncpp -I ../oj_server/include -L ../oj_server/lib -std=c++11 -lpthread -lctemplate -ljsoncpp -lmysqlclient -ldl -lssl -lcrypto

testCtemplate: testCtemplate.cc
	g++ -o $@ $^ -I ../oj_server/include -L ../oj_server/lib -std=c++11  -lctemplate -lpthread

.PHONY: clean
clean:
	rm -f testMD5 UserControlTest

