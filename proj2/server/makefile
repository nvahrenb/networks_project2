EXEC = thermd

all: thermd.c
	gcc thermd.c -o $(EXEC) -lpthread
	
debug:
	gcc thermd.c -o $(EXEC) -DDEBUG -g -lpthread
	
clean:
	rm -f $(EXEC) *~
