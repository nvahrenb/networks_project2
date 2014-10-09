EXEC = therm

all: therm.c
	gcc therm.c -o $(EXEC)
	
debug:
	gcc therm.c -o $(EXEC) -DDEBUG -g
	
clean:
	rm -f $(EXEC) *~
