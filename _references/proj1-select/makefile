all: Admin.exe Client.exe

Admin.exe: computer.o reqhandler.o queue.o
	gcc -pthread -o Admin.exe computer.o reqhandler.o queue.o

computer.o: computer.c computer.h
	gcc -c computer.c

reqhandler.o: reqhandler.c computer.h
	gcc -c reqhandler.c

queue.o: queue.c computer.h
	gcc -c queue.c

Client.exe: client.c
	gcc -o Client.exe client.c

