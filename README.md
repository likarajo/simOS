# simOS
Simulate a computer system and implement a simple Operating System that manages the resources of the computer system

## 1. Overview of a Simulated Simple OS
* The computer system has the following components: 
  * CPU (with registers)
  * memory
  * disk swap space
  * clock
* CPU drives the memory through *load* and *store* instructions, and other I/O devices (e.g., I/O for page fault).
  * These are simulated by software function calls.
* When system starts, the CPU executes the OS program
  * Loads in user programs upon submissions
  * Initiates CPU registers for a user program so that CPU executes it
  
### 1.1 Simulated Computer System
* CPU has a set of registers defined in ***simos.h***
* CPU performs computation on these registers implemented by function *cpu_execution* in ***cpu.c***
* Besides the *end-program* instruction, the corresponding datum for the instruction is fetched from memory and executed.<br>
![](/_images/instruction.png)<br>
* At the end of each instruction execution cycle, CPU checks the interrupt vector. 
  * If some interrupt bits are set, then the corresponding interrupt handling actions are performed by *handle_interrupt* in ***cpu.c***.
  * 3 interrupt bits are defined in *simos.h*. The interrupt vector is initialized to 0. After completing interrupt handling, the interrupt vector is reset to 0.<br>
 ![](/_images/interrupt.png)<br>
* Memory provides CPU the functions required during CPU execution, including *get_instruction(offset)*, *get_data(offset)*, and *put_data(offset)* in ***memory.c***.
  * The parameter offset is based on the address space of the process and has to be converted to physical memory address. This conversion is supposed to be done above the physical memory, but for convenience, we put it in physical memory.
* When a new process is submitted to the system, the system will load the program and the corresponding data into memory. 
  * So memory unit provides functions *load_instruction(...)* and *load_data(...)* for this purpose (simulating one type of direct memory access (DMA) without going through CPU).
* Terminal ***term.c** outputs a string to the monitor. 
* When CPU ***cpu.c*** processes the print instruction, it puts what is to be printed as a string and sends the string to the terminal queue.
* When a process ends, process manager **process.c** prepares an end-program message and sends it to terminal queue. At the same time, the running process is switched into the waiting state.
* Since terminal is an IO device working in parallel with CPU, we need to run terminal manager as a thread. The terminal thread goes through the printing requests in the queue and process them (print the string). Note that inserting requests to the terminal queue is done by the main thread and the terminal thread removes requests from the queue. Thus, they need to be synchronized.
* A data intensive process may use a lot of memory space for processing a large amount of data. It may not be possible to put all the pages of a process fully in memory. Swap space is used to allow the system to only keep currently needed pages in memory and put the remaining pages for a process on disk called *swap space*. We simulate the disk swap space and implement the swap space manager in ***swap.c***.
  
