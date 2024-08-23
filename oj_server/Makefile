oj_server:oj_server.cpp
	g++ -o $@ $^  -I/usr/include/mysql -L/usr/lib64/mysql -lpthread -lctemplate -ljsoncpp -lmysqlclient
.PHONY:clean
clean:
	rm -rf oj_server