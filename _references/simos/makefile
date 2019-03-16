final: simos.exe

simos.exe: system.o admin.o submit.o process.o\
           cpu.o memory.o swap.o term.o clock.o
	gcc -o simos.exe system.o admin.o submit.o process.o\
               cpu.o memory.o swap.o term.o clock.o -lpthread

system.o: system.c simos.h
	gcc -c system.c
# Initlize the system and start admin, client, term processing

admin.o: admin.c simos.h
	gcc -c admin.c
# Read admin commands and process them, drive the whole system
 
submit.o: submit.c simos.h
	gcc -c submit.c
# Interface with the client to accept client submissions.
# Call submit process in process.c.

process.o: process.c simos.h
	gcc -c process.c
# Manage the processes in the system.
# Accept client submissions and prepare the process, call memory to
# allocate memory and load program to memory, call cpu to execute processes,
# interact with clock to advance clock and set timers

cpu.o: cpu.c simos.h
	gcc -c cpu.c
# Simulate CPU in executing instructions and handling interrupts.

memory.o: memory.c simos.h
	gcc -c memory.c
# Simulate memory functions. Implement memory manager tasks

swap.o: swap.c simos.h
	gcc -c swap.c
# swap space manager for maintaining pages that cannot be loaded to memory

term.o: term.c simos.h
	gcc -c term.c
# Simulate the terminal output. Process wanting to output has to go to
# wait state and insert to terminal queue and back to ready after finishing

clock.o: clock.c simos.h
	gcc -c clock.c
# Simulate the clock. Host the advance_clock function for clock.
# The remaining functions are for timers. Users can set different timers
# and when time is up there will  timer interrupt.

clean: 
	rm *.o simos.exe swap.disk terminal.out

