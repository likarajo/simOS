#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "simos.h"

//=========================================================================
// Terminal manager is responsible for printing an output string to terminal.
// When there is a term output, the process has to be in eWait state and
// we insert both pid and the output string to the terminal queue.
// After terminal output is done, we need to put process back to ready state,
// which has to be done by the process manager.
// The terminal can only put process in endWait queue and set the interrupt.
//=========================================================================

FILE *fterm;
// terminal output file (on terminal would cause garbled output)

void terminal_output (int pid, char *outstr);

// if terminal queue is empty, wait on semaq
// for each insersion, signal semaq
// essentially, semaq.count keeps track of #req in the queue
sem_t semaq;

// for access the queue head and queue tail
sem_t mutex;

// *** ADD CODE, add semaphores to make sure 
// 1. the system will not busy looping on null terminal queue
// 2. terminal thread and main thread will not have conflict Q accesses
// two semaphores needed are defined above, add sem_init, sem_wait, sem_post 

//=========================================================================
// terminal output queue 
// implemented as a list with head and tail pointers
//=========================================================================

typedef struct TermQnodeStruct
{ int pid, type;
  char *str;
  struct TermQnodeStruct *next;
} TermQnode;

TermQnode *termQhead = NULL;
TermQnode *termQtail = NULL;

// dump terminal queue is not inside the terminal thread,
// only called by admin.c
void dump_termio_queue ()
{ TermQnode *node;

  printf ("******************** Term Queue Dump\n");
  node = termQhead;
  while (node != NULL)
    { printf ("%d, %s\n", node->pid, node->str);
      node = node->next;
    }
  printf ("\n");
}

// insert terminal queue is not inside the terminal thread, but called by
// the main thread when terminal output is needed (only in cpu.c, process.c)
void insert_termio (pid, outstr, type)
int pid, type;
char *outstr;
{ TermQnode *node;

  if (Debug) printf ("Insert term queue %d %s\n", pid, outstr);
  node = (TermQnode *) malloc (sizeof (TermQnode));
  node->pid = pid;
  node->str = outstr;
  node->type = type;
  node->next = NULL;
  if (termQtail == NULL) // termQhead would be NULL also
    { termQtail = node; termQhead = node; }
  else // insert to tail
    { termQtail->next = node; termQtail = node; }
  if (Debug) dump_termio_queue ();
}

// remove the termIO job from queue and call terminal_output for printing
// after printing, put the job to endWait list and set endWait interrupt
void handle_one_termio ()
{ TermQnode *node;

  if (Debug) dump_termio_queue ();
  if (termQhead == NULL)
  { printf ("No process in terminal queue!!!\n"); }
  else 
  { node = termQhead;
    terminal_output (node->pid, node->str);
    if (node->type != endIO)
    { insert_endWait_process (node->pid);
      set_interrupt (endWaitInterrupt);
    }   // if it is the endIO type, then job done, just clean termio queue

    if (Debug) printf ("Remove term queue %d %s\n", node->pid, node->str);
    termQhead = node->next;
    if (termQhead == NULL) termQtail = NULL;
    free (node->str); free (node);
    if (Debug) dump_termio_queue ();
  }
}


//=====================================================
// IO function, loop on handle_one_termio to process the termio requests
// This has to be a separate thread to loop on the termio_one_handler
//=====================================================

// pretent to take a certain amount of time by sleeping printTime
// output is now simply printf, but it should be sent to client terminal
void terminal_output (pid, outstr)
int pid;
char *outstr;
{
  fprintf (fterm, "%s\n", outstr);
  fflush (fterm);
  usleep (termPrintTime);
}

void *termIO ()
{
  while (systemActive) handle_one_termio ();
  printf ("TermIO loop has ended\n");
}

pthread_t termThread;

void start_terminal ()
{ int ret;

  fterm = fopen ("terminal.out", "w");
  ret = pthread_create (&termThread, NULL, termIO, NULL);
  if (ret < 0) printf ("TermIO thread creation problem\n");
  else printf ("TermIO thread has been created successsfully\n");
}

void end_terminal ()
{ int ret;

  fclose (fterm);
      // no problem if we post additional signals becuase
      // the null queue is always checked anyway
  ret = pthread_join (termThread, NULL);
  printf ("TermIO thread has terminated %d\n", ret);
}


