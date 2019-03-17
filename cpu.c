#include <stdio.h>
#include <stdlib.h>
#include "simos.h"


#define OPload 2
#define OPadd 3
#define OPmul 4
#define OPifgo 5
#define OPstore 6
#define OPprint 7
#define OPsleep 8
#define OPend 1


void initialize_cpu ()
{ // Generally, cpu goes to a fix location to fetch and execute OS
  CPU.interruptV = 0;
  CPU.numCycles = 0;
}

void dump_registers ()
{ 
  printf ("*************************** Register Dump\n");
  printf ("Pid = %d\n", CPU.Pid);
  printf ("PC = %d\n", CPU.PC);
  printf ("AC = %.2f\n", CPU.AC);
  printf ("MBR = %.2f\n", CPU.MBR);
  printf ("IRopcode = %d\n", CPU.IRopcode);
  printf ("IRoperand = %d\n", CPU.IRoperand);
  printf ("Mbase = %d\n", CPU.Mbase);
  printf ("MDbase = %d\n", CPU.MDbase);
  printf ("Mbound = %d\n", CPU.Mbound);
  printf ("exeStatus = %d\n", CPU.exeStatus);
  printf ("InterruptV = %x\n", CPU.interruptV);
  printf ("numCycles = %d\n", CPU.numCycles);
}

void set_interrupt (bit)
unsigned bit;
{
  CPU.interruptV = CPU.interruptV | bit;
}

void handle_interrupt ()
{
  if ((CPU.interruptV & ageInterrupt) == ageInterrupt)
    memory_agescan ();
  if ((CPU.interruptV & endWaitInterrupt) == endWaitInterrupt)
    endWait_moveto_ready ();  
     // interrupt may overwrite, move all IO done processes (maybe more than 1)
  if ((CPU.interruptV & tqInterrupt) == tqInterrupt)
    if (CPU.exeStatus == eRun) CPU.exeStatus = eReady;
      // need to do this last, in case exeStatus is changed for other reasons

  // all interrupt bits have been taken care of, reset vector
  CPU.interruptV = 0;
  if (Debug) 
    printf ("Interrup handler: pid = %d; interrupt = %x; exeStatus = %d\n",
            CPU.Pid, CPU.interruptV, CPU.exeStatus); 
}


void cpu_execution ()
{ float sum, temp;
  int mret, gotoaddr;

  // perform all memory fetches, analyze memory conditions all here
  while (CPU.exeStatus == eRun)
  { mret = get_instruction (CPU.PC);
    if (mret == mError) CPU.exeStatus = eError;
    else if (mret == mPFault) CPU.exeStatus = ePFault;
    else if (CPU.IRopcode != OPend && CPU.IRopcode != OPsleep)
      // fetch data, but exclude OPend and OPsleep, which has no data
    { mret = get_data (CPU.IRoperand); 
      if (mret == mError) CPU.exeStatus = eError;
      else if (mret == mPFault) CPU.exeStatus = ePFault;
      else if (CPU.IRopcode == OPifgo)
      { mret = get_instruction (CPU.PC+1);
        if (mret == mError) CPU.exeStatus = eError;
        else if (mret == mPFault) CPU.exeStatus = ePFault;
        else { CPU.PC++; CPU.IRopcode = OPifgo; }
        // ifgo is different from other instructions, it has two words
        //   test variable is in memory, memory addr is in the 1st word
        //      get_data above gets it in MBR
        //   goto addr is in the operand field of the second word
        //     we use get_instruction again to get it as the operand
        // we also advance PC to make it looks like ifgo only has 1 word
      }
    }

    if (Debug)
    { printf ("Process %d executing: ", CPU.Pid);
      printf ("PC=%d, OPcode=%d, Operand=%d, AC=%.2f, MBR=%.2f\n",
              CPU.PC, CPU.IRopcode, CPU.IRoperand, CPU.AC, CPU.MBR);
    }

    // if it is eError or eEnd, then does not matter 
    // if it is page fault, then AC, PC should not be changed
    // because the instruction should be re-executed
    if (CPU.exeStatus == eRun)
    { switch (CPU.IRopcode)
      { case OPload:
          // *** ADD CODE for the instruction
          CPU.AC = CPU.MBR;
          break;
        case OPadd:
          // *** ADD CODE for the instruction
          CPU.AC = CPU.AC + CPU.MBR;
          break;
        case OPmul:
          // *** ADD CODE for the instruction
          int temp = CPU.MBR;
          int res = 0;
          while (temp > 0) {
            res = res+temp;
            temp = temp-1;
          }
          CPU.AC = CPU.AC + res;
          break;
        case OPifgo:  // conditional goto, need two instruction words
          // earlier, we got test variable in MBR and goto addr in IRoperand
            // Note: PC will be increased by 1, so need to set it to 1 less
          // *** ADD CODE for the instruction
          if (CPU.MBR > 0){
            CPU.PC = CPU.IRoperand - 1;
          }
          break;
        case OPstore:
          put_data (CPU.IRoperand);
          break;
        case OPprint:
          // print content to a printing string
          // send the string to terminal for printing
          // *** ADD CODE for the instruction
          sprintf (str, "%.2f\n", CPU.MBR);
          spret = spool (str);
          if (spret == spError){
            CPU.exeStatus = eError;
          }
          break;
        case OPsleep:
          // *** ADD CODE for adding a timer event
          add_timer (CPU.IRoperand, CPU.Pid, actReadyInterrupt, oneTimeTimer);
          CPU.exeStatus = eWait;
          break;
        case OPend:
          CPU.exeStatus = eEnd;
          break;
        default:
          printf ("Illegitimate OPcode in process %d\n", CPU.Pid);
          CPU.exeStatus = eError;
      }
      CPU.PC++;
    }

    // since we don't have clock, we use instruction cycle as the clock
    // no matter whether there is a page fault or an error,
    // should handle clock increment and interrupt
    advance_clock ();
    if (CPU.interruptV != 0) handle_interrupt ();
  }
}

