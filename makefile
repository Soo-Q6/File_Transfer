CC=gcc
all:server  client
.PHONY:all

server:HW3_0540112_Ser.o ser.o
	$(CC) -o server HW3_0540112_Ser.o ser.o -pthread
client:HW3_0540112_Cli.o cli.o
	$(CC) -o client HW3_0540112_Cli.o cli.o
HW3_0540112_Ser.o:HW3_0540112_Ser.c ser.c ser.h
	$(CC) -c HW3_0540112_Ser.c ser.c 
HW3_0540112_Cli.o:HW3_0540112_Cli.c cli.c cli.h
	$(CC) -c HW3_0540112_Cli.c cli.c 

.PHONY:clean
clean:
	rm *.o server  client	
