#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "simos.h"


//======================================================================
// This module handles swap space management.
// It has the simulated disk and swamp manager.
// First part is for the simulated disk to read/write pages.
//======================================================================

int diskfd;
int swapspaceSize;
int PswapSize;
int pagedataSize;

//===================================================
// This is the simulated disk, including disk read, write, dump.
// The unit is a page
//===================================================

// each process has a fix-sized swap space, its page count starts from 0
// first 2 processes: OS=0, idle=1, have no swap space (not true in real os)
// Since we mainly deal with integer content, here buf is an integer buffer
int read_swap_page (pid, page, buf)
int *buf;
int pid, page;
{ int location, ret, retsize, k;

  if (pid < 2 || pid > maxProcess) 
  { printf ("Error: Incorrect pid for disk read: %d\n", pid); 
    return (-1);
  }
  location = (pid-2) * PswapSize + page*pagedataSize;
  ret = lseek (diskfd, location, SEEK_SET);
  if (ret < 0) perror ("Error lseek in read: \n");
  retsize = read (diskfd, (char *)buf, pagedataSize);
  if (retsize != pagedataSize) 
  { printf ("Error: Disk read returned incorrect size: %d\n", retsize); 
    exit(-1);
  }
  usleep (diskRWtime);
}

int write_swap_page (pid, page, buf)
int *buf;
int pid, page;
{ int location, ret, retsize;

  if (pid < 2 || pid > maxProcess) 
  { printf ("Error: Incorrect pid for disk write: %d\n", pid); 
    return (-1);
  }
  location = (pid-2) * PswapSize + page*pagedataSize;
  ret = lseek (diskfd, location, SEEK_SET);
  if (ret < 0) perror ("Error lseek in write: \n");
  retsize = write (diskfd, (char *)buf, pagedataSize);
  if (retsize != pagedataSize) 
    { printf ("Error: Disk read returned incorrect size: %d\n", retsize); 
      exit(-1);
    }
  usleep (diskRWtime);
}

int dump_swap_page (pid, page)
int pid, page;
{ int location, ret, retsize, k;
  int buf[pageSize];

  if (pid < 2 || pid > maxProcess) 
  { printf ("Error: Incorrect pid for disk dump: %d\n", pid); 
    return (-1);
  }
  location = (pid-2) * PswapSize + page*pagedataSize;
  ret = lseek (diskfd, location, SEEK_SET);
  //printf ("loc %d %d %d, size %d\n", pid, page, location, pagedataSize);
  if (ret < 0) perror ("Error lseek in dump: \n");
  retsize = read (diskfd, (char *)buf, pagedataSize);
  if (retsize != pagedataSize) 
  { printf ("Error: Disk dump read incorrect size: %d\n", retsize); 
    exit(-1);
  }
  printf ("Content of process %d page %d:\n", pid, page);
  for (k=0; k<pageSize; k++) printf ("%d ", buf[k]);
  printf ("\n");
}

int dump_process_swap_page (pid)
int pid;
{ int i;
  for (i=0; i<maxPpages; i++) dump_swap_page (pid, i);
}

// open the file with the swap space size, initialize content to 0
initialize_swap_space ()
{ int ret, i, j, k;
  int buf[pageSize];

  swapspaceSize = maxProcess*maxPpages*pageSize*dataSize;
  PswapSize = maxPpages*pageSize*dataSize;
  pagedataSize = pageSize*dataSize;

  diskfd = open ("swap.disk", O_RDWR | O_CREAT, 0600);
  if (diskfd < 0) perror ("Error open: ");
  ret = lseek (diskfd, swapspaceSize, SEEK_SET); 
  if (ret < 0) perror ("Error lseek in open: ");
  for (i=2; i<maxProcess; i++)
    for (j=0; j<maxPpages; j++)
    { for (k=0; k<pageSize; k++) buf[k]=0;
      write_swap_page (i, j, buf);
    }
    // last parameter is the origin, offset from the origin
    // SEEK_SET: 0, SEEK_CUR: from current position, SEEK_END: from eof
}


//===================================================
// Here is the swap space manager. 
//===================================================
// When a process address to be read/written is not in the memory,
// meory raises a page fault and process it (in kernel mode).
// We implement this by cheating a bit.
// We do not perform context switch right away and switch to OS.
// We simply let OS do the processing.
// OS decides whether there is free memory frame, if so, use one.
// If no free memory, then call select_aged_page to free up memory.
// In either case, proceed to insert the page fault req to swap queue
// to let the swap manager bring in the page
//===================================================

// pidin, pagein, inbuf: for the page with PF, needs to be brought in
// pidout, pageout, outbuf: for the page to be swapped out
// if there is free memory or the out page is not dirty,
//   then pidout and pageout should be set to -1
// inbuf and outbuf can be the actual memory page

// *** ADD CODE for all functions here 
// This is for the last project

void insert_swapQ (pidin, pagein, inbuf, pidout, pageout, outbuf)
int pidin, pagein, pidout, pageout;
int *inbuf, *outbuf;
{
  // simply add all parameters into swap queue
}

void process_one_swap ()
{ // get one request from the head of the swap queue and process it
  // if (pid >= 2 && page >= 0) error
    // call write_swap_page to write the dirty page out 
  // call read_swap_page to read in the needed page
  // after finishing return the process to ready queue and set interrup
}

void *process_swapQ ()
{ // while system is active and swap queue is not empty,
  // process the requests in swap queue
}

void dump_swapQ ()
{
  printf ("******************** Swap Queue Dump\n");
  // dump queue info
}

void start_swap_manager ()
{
  initialize_swap_space ();
  // initialize other needed parameters
  // create a thread to perform the swap manager tasks 
  // i.e., create a thread that executes process_swapQ
  printf ("Swap space managr has been activated.\n");
}

void end_swap_manager ()
{
  close (diskfd);
  // wait for thread to finish by calling pthread_join
  printf ("Swap space managr has terminated successfully.\n");
}


