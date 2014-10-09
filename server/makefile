EXEC = thermd

all: thermd.c
	gcc thermd.c -o $(EXEC)
	
debug:
	gcc thermd.c -o $(EXEC) -DDEBUG -g
	
clean:
	rm -f $(EXEC) *~
