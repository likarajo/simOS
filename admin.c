#include <stdio.h>
#include <stdlib.h>
#include "simos.h"


void process_admin_command ()
{ char action[10];
  char fname[100];

  while (systemActive)
  { printf ("command> ");
    scanf ("%s", &action);
    if (Debug) printf ("Command is %c\n", action[0]);
    // only first action character counts, discard remainder
    switch (action[0])
    {
      /*case 's':  // submit
         one_submission ();
         break;*/
      case 'x':  // execute
        execute_process ();
        break;
      case 'r':  // dump register
        dump_registers ();
        break;
      case 'q':  // dump ready queue and list of processes completed IO
        dump_ready_queue ();
        dump_endWait_list ();
        break;
      case 'p':   // dump PCB
        dump_PCB_list ();
        break;
      case 'm':   // dump Memory
        dump_PCB_memory ();
        break;
      case 'e':   // dump events in timer
        dump_events ();
        break;
      case 'd':   // dump terminal IO queue
        dump_termio_queue ();
        dump_swapQ ();
        break;
      case 'T':  // Terminate, do nothing, terminate in while loop
        systemActive = 0;
        exit(0);
        break;
      default:   // can be used to yield to client submission input
        printf ("Incorrect command!!!\n");
    }
  }
  printf ("Admin command processing loop ended!\n");
}


