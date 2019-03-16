#include <stdio.h>
#include "simos.h"

//======================================================================
// Our memory addressing is in WORDs, because of the Memory structure def
// - so all addressing is computed in words
//======================================================================

#define fixMsize 64  // each process has a fixed memory size
                     // first part of the process memory is for program
                     // remaining is for data
                     // this size is actually 256*4 = 1024B
#define dataSize 4   // each memroy unit is of 4 bytes
#define OSsize fixMsize // first physical memory segment is for OS

typedef union
{ float mData;
  int mInstr;
} mType;

#define memSize maxPCB*fixMsize
mType Memory[memSize];

#define opcodeShift 24
#define operandMask 0x00ffffff


//============================
// Our memory implementation is a mix of memory manager and physical memory.
// get_instr, put_instr, get_data, put_data are the physical memory operations
//   for instr, instr is fetched into registers: IRopcode and IRoperand
//   for data, data is fetched into registers: MBR (need to retain AC value)
//             but stored directly from AC
//   -- one reason is because instr and data do not have the same types
//      also for convenience
// allocate_memory, deallocate_memory are pure memory manager activities
//============================


//==========================================
// run time memory access operations, called by cpu.c
//==========================================

int check_address (maddr)
int maddr;
{ 
  if (Debug) printf ("Memory access: %d; ", maddr);
  if (maddr < OSsize)
  { printf ("Process %d accesses %d. In OS region!\n", CPU.Pid, maddr);
    return (mError);
  }
  else if (maddr > CPU.Mbound)
  { printf ("Process %d accesses %d. Outside addr space!\n", CPU.Pid, maddr);
    return (mError);
  }
  else if (maddr >= memSize) 
  { printf ("Process %d accesses %d. Outside memory!\n", CPU.Pid, maddr);
    return (mError);
  }
  else 
  { if (Debug) printf ("content = %x, %.2f\n",
                        Memory[maddr].mData, Memory[maddr].mInstr);
    return (mNormal);
  }
}

int get_data (offset)
int offset;
{ int maddr; 

  maddr = CPU.MDbase + offset;
  if (check_address (maddr) == mError) return (mError);
  else
  { CPU.MBR = Memory[maddr].mData;
    return (mNormal);
  }
}

int put_data (offset)
int offset;
{ int maddr; 

  maddr = CPU.MDbase + offset;
  if (check_address (maddr) == mError) return (mError);
  else
  { Memory[maddr].mData = CPU.AC;
    return (mNormal);
  }
}

int get_instruction (offset)
int offset;
{ int maddr, instr; 

  maddr = CPU.Mbase + offset;
  if (check_address (maddr) == mError) return (mError);
  else
  { instr = Memory[maddr].mInstr;
    CPU.IRopcode = instr >> opcodeShift; 
    CPU.IRoperand = instr & operandMask;
    return (mNormal);
  }
}

//==========================================
// load instructions and data into memory 
// a specific pid is needed for loading, since registers are not for this pid
// called by process.c
//==========================================

int check_load_address (pid, maddr)
int pid, maddr;
{ 
  if (maddr < OSsize)
  { printf ("Process %d accesses %d. In OS region!\n", pid, maddr);
    return (mError);
  }
  else if (maddr > PCB[pid]->Mbound)
  { printf ("Process %d accesses %d. Outside address space!\n", pid, maddr);
    return (mError);
  }
  else if (maddr >= memSize) 
  { printf ("Process %d accesses %d. Outside memory!\n", pid, maddr);
    return (mError);
  }
  else return (mNormal);
}

int load_instruction (pid, offset, opcode, operand)
int pid, offset, opcode, operand;
{ int maddr;

  maddr = PCB[pid]->Mbase + offset;
  if (check_load_address (pid, maddr) == mError) return (mError);
  else
  { opcode = opcode << opcodeShift;
    operand = operand & operandMask;
    Memory[maddr].mInstr = opcode | operand;
    return (mNormal);
  }
}

int load_data (pid, offset, data)
int pid, offset;
float data;
{ int maddr;

  maddr = PCB[pid]->MDbase + offset;
  if (check_load_address (pid, maddr) == mError) return (mError);
  else
  { Memory[maddr].mData = data;
    return (mNormal);
  }
}


//==========================================
// Basic memory management functions 
//==========================================

int allocate_memory (pid, msize, numinstr)
int pid, msize, numinstr;
{
  if (pid >= maxProcess)
  { printf ("Invalid pid: %d\n", pid); return(mError); }
  else if (msize > fixMsize)
  { printf ("Invalid memory size %d for process %d\n", msize, pid);
    return(mError);                                                }
  else
  { PCB[pid]->Mbase = pid * fixMsize;
    PCB[pid]->Mbound = PCB[pid]->Mbase + msize - 1;
    PCB[pid]->MDbase = PCB[pid]->Mbase + numinstr;
          // here we let the first part of process memory be instructions
          // MDbase starts after numinstr instructions
    return (mNormal);
  }
}

// Due to our simple allocation, nothing to do for deallocation
// in paging scheme, need to return the pages to free list
int free_memory (pid)
int pid;
{
  return (mNormal);
}

void dump_memory (pid)
int pid;
{ int i, start;

  printf ("************ Instruction Memory Dump for Process %d\n", pid);
  start = PCB[pid]->Mbase;
  for (i=0; i<PCB[pid]->numInstr; i++)
    printf ("%x ", Memory[start+i].mInstr);
  printf ("\n");

  printf ("************ Data Memory Dump for Process %d\n", pid);
  start = PCB[pid]->MDbase;
  for (i=0; i<PCB[pid]->numStaticData; i++)
    printf ("%.2f ", Memory[start+i].mData);
  printf ("\n");
}


//==========================================
// paging related memory management functions 
//==========================================
// *** ADD CODE to this remaining part
// This is for the last project

// build memory page structure metadata
// Each page has a pointer pointing to the memory array
// Each page also maintains an age byte and a dirty bit

void memory_agescan ()
{ // go through memory pages to update the age field
}

void start_periodical_page_scan ()
{ add_timer (periodAgeScan, osPid, actAgeInterrupt, periodAgeScan);
} 

void select_aged_page (pid, page, dirty)
int *pid, *page, *dirty;
{ // select the aged page based on age vector
}

int get_free_page ()
{
}

int * get_memoryPtr (pid, page)
int pid, page;
{ // return the pointer to memory page
}

int page_fault_handler (pid, page)
int pid, page;
{ int swappid, swappage, dirty;
  int *inbuf, *outbuf;

  swappid = -1;
  swappage = get_free_page ();
  if (swappage < 0)
  { select_aged_page (&swappid, &swappage, &dirty);
    if (&swappid < 0) return (mError);
  }
  // update page table
  inbuf = get_memoryPtr (pid, page);
  outbuf = get_memoryPtr (swappid, swappage);
  if (!dirty)  // no need to write back
    { swappid = -1; swappage = -1; }
  insert_swapQ (pid, page, inbuf, swappid, swappage, outbuf);
}

void initialize_memory_manager ()
{ // initialize free page list
  // in the begining, all memory pages are free pages
  start_periodical_page_scan ();
}

