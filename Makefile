.PHONY:all
all:server
	                                                                                                                                       
server:server.cpp comm.cpp
	g++ -o $@ $^ -lpthread
		  
.PHONY:clean
clean:
	rm -f server
	

	
	

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
