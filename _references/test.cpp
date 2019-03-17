/*Name : Madhusudan Pranav Venugopal
UTD ID: 2021163441
netid: mxv131930
CS5348
Operating Systems Project 3
Simulated Operating Systems

*/

// Header files
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std; // Using standard naming onventions

sem_t IObegin; // Semaphore used by main thread to indicate to io thread thread when page fault occurs

pthread_t iothread1; // thread declartion of io thread


// Defination of terms used in the project code
#define idlePid 1
#define maxCPUcycles 1024*1024*1024// = 2^30
#define Debug 1
#define OPifgo 5
#define idleMsize 3
#define idleNinstr 2
#define opcodeShift 24
#define operandMask 0x00ffffff
#define mNormal 0
#define mError -1
#define mPFault 3
#define osPid 0
#define idlePid 1
#define eRun 1
#define eReady 2
#define ePFault 3
#define eWait 4
#define eEnd 0
#define eError -1
#define oneTimeTimer 0

#define prError -1
#define spNormal 0
#define spError -1

#define prNormal 0
#define actTQinterrupt 1
#define actAgeInterrupt 2
#define actReadyInterrupt 3
#define actNull 0
#define OPload 2
#define OPadd 3
#define OPsum 4
#define OPifgo 5
#define OPstore 6
#define OPprint 7
#define OPsleep 8
#define OPend 1
int currentPid=2;
int done=0;
#define nullReady 0
#define maxReady 100
int swaplist;
// The ready queue capacity, gives a limit on number of
// ready processes the system can handle
typedef struct doneWaitNode
{ int pid;
  struct doneWaitNode *next;
} doneWaitNode;
#define tqInterrupt 1      // for time quantum
#define ageInterrupt 2     // for age scan
#define doneWaitInterrupt 4  // for any IO completion, including page fault
doneWaitNode *doneWaitHead = NULL;
doneWaitNode *doneWaitTail = NULL;

int readyList[maxReady];
int readyCount = 0;
int readyHead = 0;
int readyTail = 0;
// head got executed next, insert a new process to tail

void insert_ready(int pid)
{
  if (readyCount >= maxReady)
    { printf ("Ready queue is full, process rejected!!!\n"); }
  else { readyList[readyTail] = pid;
    readyCount++;
    readyTail = (readyTail+1) % maxReady;
  }
}
int get_ready ()
{
  if (readyCount == 0)
    { printf ("No ready process now!!!\n"); return (nullReady); }
  else return (readyList[readyHead]);
}

void remove_ready ()
{
  if (readyCount == 0)
    { printf ("Incorrect removal of ready list!!!\n"); exit(-1); }
  readyCount--;
  readyHead = (readyHead+1) % maxReady;
}

// Declartion of the variables used in the program code
typedef unsigned *genericPtr;
int Observe;
int pageSize, memSize, swapSize, OSmemSize;
int periodAgeScan;
int cpuQuantum;
int idleQuantum;
int spoolPsize;
int noofprocess=2;
int noofframes;
int numi;

// structure for the memory frame
struct  memoryframe{
  int frameno;
  int mInstr;
  float mData;
  int dirtybit;
  int age;
}*mf;

// structure to store data while being inserted into io thread queue
struct ioqdata{
  memoryframe *m1,*m2,*m3;
  int task;
  int pid;
  int fno;
  int swappedpid;
  int swappedoffset;
  int swappedframe;
};

// class for the iothread queue
class ioqueue{
private:
  ioqdata q1[200];
  int head;
  int tail;
public:
  void enqueue(ioqdata d1)
  {
    q1[tail++]= d1;
  }
  ioqdata dequeue()
  {
    return q1[head++];
  }
}ioq;

int freelistcount,swaplistcount,poffset[2000];

// structure to manage the freelist and swaplist frames
struct listnode{
  int frame;
  listnode *next;
}*flstart=NULL,*slstart=NULL;


// Class for the address table
class addressentry{
private:
  int frameno;
  memoryframe *address;
public:
  addressentry(int key1,memoryframe *address1)
  {
    frameno=key1;
    address=address1;
  }

  int getframeno()
  {
    return frameno;
  }
  memoryframe* getaddress()
  {
    return address;
  }
};

class addresstable{
private:
  addressentry **table;
  int tablesize;
public:
  addresstable(int tablesize1)
  {
    tablesize=tablesize1;
    table= new addressentry *[tablesize];
    for(int i=0; i<tablesize;i++)
      table[i]=NULL;
  }
  memoryframe* get(int frameno)
  {
    memoryframe *m;
    m= table[frameno]->getaddress();
    return m;
  }
  void put(int frameno, memoryframe *address )
  {
    if(table[frameno]!=NULL)
      delete table[frameno];
    table[frameno] = new addressentry(frameno,address);
  }
  ~addresstable()
  {
    for(int i=0;i<tablesize;i++)
      {
	if(table[i]!=NULL)
	  delete table[i];
      }
    delete[] table;
  }
}*at;
memoryframe* computeaddress(int frameno)
{
  memoryframe* p;
  p=at->get(frameno);
  if(Debug==1)
    {
      cout<<endl<<" Computing address for frame no :"<<frameno;
      cout<<" Address is "<<p;
    }
  return p;
}

// class for each row entry of the page table
class page{

private:
  int pageno;
  int frameno;
public:
  page(int offset,int frameno1)
  {
    pageno=offset;
    frameno=frameno1;
  }
  int getpageno()
  {
    return pageno;
  }
  int getframeno()
  {
    return frameno;
  }
};

//  class for page table of each process
class pagetable{
private:
  page **table;
  int tablesize;
public:
  pagetable()
  {
    tablesize=100;
    table = new page*[100];
  }
  int get(int offset)
  {
    if(table[offset]== NULL)
      return -1;
    else
      return table[offset]->getframeno();
  }
  void put(int offset, int frameno)
  {
    if(table[offset]!=NULL)
      delete table[offset];
    table[offset]= new page(offset,frameno);
  }
  void deletepage(int offset)
  {
    table[offset]= NULL;
  }
  void showpagetable()
  {
    int spgno=0;
    for(int i=0;i<tablesize;i++)
      {
	if(table[i]!=NULL)
	  {
	    cout<<endl<<"Pageno : "<<spgno++;
	    cout<<" Frame no : "<<get(i);
	    memoryframe *p;
	    p= computeaddress(get(i));
	    cout<<endl<<" Frame address :"<<p ;
	    cout<<endl<<" Frame content : "<<p->mData||p->mInstr;
	    cout<<endl<<" Age:"<<p->age;
	    cout<<endl<<" Dirty bit:"<<p->dirtybit;
	  }
      }
  }
  int  getoffset( int dframe)
  {
    for(int i=0;i<tablesize;i++)
      {
	if(get(i)==dframe)
	  {
	    return table[i]->getpageno();
	  }
      }
    return -1;
  }
  ~pagetable()
  {
    for(int i=0;i<tablesize;i++)
      {
	delete table[i];
      }
    delete[] table;

  }
}ptg[2000];

//structure for the CPU
struct CPU1
{ int Pid;
  int PC;
  float AC;
  float MBR;
  int IRopcode;
  int IRoperand;
  int Mbase;
  int MDbase;
  int Mbound;
  char *spoolPtr;
  int spoolPos;
  int exeStatus;
  unsigned interruptV;
  int numCycles;  // this is a global register, not for each process
} CPU;
void set_interrupt (unsigned bit)
{
  CPU.interruptV = CPU.interruptV | bit;
  __sync_or_and_fetch (&CPU.interruptV, bit);
}

//  Structure for the PCB of each process
struct typePCB
{ int Pid;
  int PC;
  float AC;
  int Mbase;
  int MDbase;
  int Mbound;
  char *spoolPtr;
  int spoolPos;
  int exeStatus;
  int numInstr;
  int numStaticData;
  int numData;
  // numData is not in use anywhere
  // it is useful if there is dynamic space used during run time
  // but here we do not consider it
  int timeUsed;
};
typePCB PCB[2000];

// Structure  for node for timer
struct eventNode
{ long long time;
  int pid;
  int act;
  int recurP;
  eventNode *left, *right;
  eventNode *parent;
} *eventTree, *eventHead;

// Declaration of the functions used in the project
void freelistinsert(int i);
int freelistget();
void freelistdisplay();
void swaplistinsert(int i);
int swaplistget();
void swaplistdisplay();
int calculateframe(int pid, int offset);
int frameswapping(int pid,int offset);
int pagefaultcalculation(int pid,int offset);
int get_data(int offset);
int get_instruction(int offset);
int put_data(int offset);
int load_data(int pid,int i,float data);
int load_instruction(int pid,int i,int opcode,int operand);
int allocate_memory(int pid,int msize,int numinstr);
void initialize_memory();
int get_PCB();
void insert_doneWait_process(int pid);
void list_events(eventNode *event);
void remove_eventhead();
void dump_events();
void dump_donewait_list();
void doneWait_moveto_ready();
void insert_event(eventNode *event);
void check_timer();
void deactivate_timer(genericPtr castedevent);
void dump_memory(int pid);
void init_pagefault_handler(int pid);
void submit_process(char *fname);
void process_command();
void *iothread(void *arg);
void init_idle_process();
void initialize_eventtree();
void initialize_timer();
genericPtr add_timer(int time,int pid,int action,int recurperiod);
void initialize_system();
void clean_process(int pid);
void end_process(int pid);
void free_PCB(int pid);
void memory_agescan();
void handle_interrupt();
void increase_numcycles();
void cpu_execution();
void execute_process();
void dump_registers();
void dump_ready_queue();
void dump_spool(int pid);
int spool(char *str);
void allocate_spool(int pid);
void free_spool(int pid);
void print_spool(int pid);
int printer(int pid,int status,char *prstr,int len);
void initialize_cpu();
void context_in(int pid);
void context_out(int pid);
void dump_PCB(int pid);


void freelistinsert(int i)
{
  listnode *newnode,*currentnode;
  newnode= new listnode;
  newnode->frame= i;
  newnode->next= NULL;

  if(flstart==NULL)
    {
      flstart=newnode;
      currentnode=newnode;
    }
  else
    {
      currentnode->next= newnode;
      currentnode= newnode;
    }
  freelistcount++;
}
int freelistget()
{
  listnode *newnode;
  newnode= flstart;
  flstart= flstart->next;
  freelistcount--;
  return newnode->frame;
}
void freelistdisplay()
{
  cout<<endl<<" The list of free frames in  the memory are :";
  listnode *newnode;
  newnode= flstart;
  while(newnode!=NULL)
    {
      cout<<endl<<" Frame no : "<<newnode->frame;
      if(Debug==1)
cout<<endl<<"  Memory Address :"<<computeaddress(newnode->frame);

      newnode=newnode->next;
    }
}

void swaplistinsert(int i)
{
  listnode *newnode,*currentnode;
  newnode= new listnode;
  newnode->frame= i;
  newnode->next= NULL;

  if(slstart==NULL)
    {
      slstart=newnode;
      currentnode=newnode;
    }
  else
    {
      currentnode->next= newnode;
      currentnode= newnode;
    }
  swaplistcount++;
}
int swaplistget()
{
  listnode *newnode;
  newnode= slstart;
  slstart= slstart->next;
  swaplistcount--;
  return newnode->frame;
}
void swaplistdisplay()
{
  cout<<endl<<" The list of free frames in  the swap list are :";
  listnode *newnode;
  newnode= slstart;
  while(newnode!=NULL)
    {
      cout<<endl<<" Frame no : "<<newnode->frame<<"  Memory Address :"<<computeaddress(newnode->frame);

      newnode=newnode->next;
    }
}
int calculateframe(int pid,int offset)
{
  int ret;
  ret=ptg[pid].get(offset);
  if(ret!=-1)
    return ret;
  else
    {
      cout<<endl<<"Page Fault!";
      poffset[pid]=offset;
      ret= mPFault;
      return ret;
    }
}
int frameswapping(int pid,int offset)
{
  ioqdata data;
  memoryframe *m1,*m2,*m3;
  int swappedpid,swappedframe,swappedoffset;

  int i,tempframe,temppid,tempage=128;
  for (int i=0;i<memSize*8/pageSize;i++)
    if(mf[i].age<=tempage)
      {
	tempframe=mf[i].frameno;
      }
  swappedframe=tempframe;
  if(Debug)
  cout<<endl<<" The swapped out frame is "<<swappedframe;

  for(i=0;i<noofprocess;i++)
    {
      if(tempframe>=PCB[pid].Mbase && tempframe<=PCB[pid].Mbound)
	{
	  swappedpid=pid;
	  break;
	}
    }
  if(Debug)
    cout<<endl<<" The swapped out process is "<<swappedpid;
  m2= computeaddress(swappedframe);
  if(m2->dirtybit !=0)
    {
      swappedoffset=ptg[swappedpid].getoffset(swappedframe);
      m3= computeaddress(PCB[pid].Mbase+swappedoffset);
      data.m3=m3;
      data.m2=m2;
      data.swappedpid=swappedpid;
      data.swappedoffset=swappedoffset;
      data.task=2;
      data.pid=pid;

      ioq.enqueue(data);
      sem_post(&IObegin);
    }
  else
    {
      data.m2=m2;
      data.swappedoffset=swappedoffset;
      data.swappedframe=swappedframe;
      data.swappedpid=swappedpid;
      data.m3=m3;
      data.pid=pid;
      ioq.enqueue(data);
      sem_post(&IObegin);
    }
  return swappedframe;
}
int pagefaultcalculation(int pid,int offset)
{
  ioqdata data;
  int fno;
  memoryframe *m1,*m2;
  if(freelistcount>0)
    {
      fno=freelistget();
      m1= computeaddress(fno);
      m2= computeaddress(PCB[pid].Mbase+offset);
      poffset[pid]=offset;
      data.m1=m1;
      data.m2=m2;
      data.task=1;
      data.pid=pid;
      data.fno=fno;
      data.pid=pid;

      ioq.enqueue(data);
      sem_post(&IObegin);
      return fno;

    }
  else
    {
      fno= frameswapping(pid,offset);
      return fno;
    }
}


int get_data(int offset)
{
  memoryframe *m;
  int fno;
  fno= calculateframe(CPU.Pid,offset);
  if(fno==mPFault)
    return (mPFault);
  m= computeaddress(fno);
  CPU.MBR=m->mData;
  m->age=128;
  return(mNormal);
}

int get_instruction(int offset)
{
  memoryframe *m;
  int fno,instr;
  fno= calculateframe(CPU.Pid,offset);
  if(fno==mPFault)
    return (mPFault);
  m= computeaddress(fno);
  instr = m->mInstr;
  CPU.IRopcode = instr >> opcodeShift;
  CPU.IRoperand = instr & operandMask;
  return(mNormal);
}

int put_data(int offset)
{
  memoryframe *m;
  int fno;
  fno= calculateframe(CPU.Pid,offset);
  m= computeaddress(fno);
  m->mData= CPU.AC;
  m->dirtybit=1;
  m->age=128;
  return(mNormal);
}

int load_data(int pid,int i,float data)
{
  memoryframe *m;
  int fno;
  fno= swaplistget();
  m= computeaddress(fno);
  m->mData= data;
  if(i== PCB[pid].numInstr)
    PCB[pid].MDbase=fno;
  if(i==(PCB[pid].numData+PCB[pid].numInstr-1))
    PCB[pid].Mbound=fno;
  return (mNormal);
}
int load_instruction(int pid,int i, int opcode, int operand)
{
  memoryframe *m;
  int fno;
  fno= swaplistget();
  if(i==0)
    PCB[pid].Mbase=fno;
  m= computeaddress(fno);
  opcode = opcode << opcodeShift;
  operand = operand & operandMask;
  m->mInstr=opcode|operand;
  return (mNormal);
}



int allocate_memory(int pid,int msize,int numinstr)
{
  memoryframe *m1,*m2;
  int pagetablesize = msize*8/pageSize;
  //ptg[pid]=new pagetable;

  int fno=freelistget();
  m1= computeaddress(fno);
  m2= computeaddress(PCB[pid].Mbase);
  m1->mData= m2->mData;
  m1->mInstr= m2->mInstr;
  ptg[pid].put(0,fno);
  return (mNormal);
}
void initialize_memory()
{
  noofframes= memSize*8/pageSize+swapSize*8/pageSize;
  mf = new memoryframe[noofframes];
  memoryframe *m;
  for(int i=0;i<noofframes;i++)
    mf[i].frameno=i;
  at= new addresstable(noofframes);
  for(int i=0;i<noofframes;i++)
    {
      m= &mf[i];
      at->put(i,m);
    }
  for(int i=OSmemSize/pageSize;i<swapSize*8/pageSize;i++)
    swaplistinsert(i);
  for(int i=swapSize*8/pageSize;i<noofframes;i++)
    freelistinsert(i);
  add_timer (periodAgeScan, osPid, actAgeInterrupt, periodAgeScan);
}


int get_PCB()
{
  int pid=currentPid;
  currentPid++;
  PCB[pid].Pid=pid;
  return(pid);
}
void insert_doneWait_process (int pid)
{ doneWaitNode *node;

  node = (doneWaitNode *) malloc (sizeof (doneWaitNode));
  node->pid = pid;
  node->next = NULL;
  if (doneWaitTail == NULL) // doneWaitHead would be NULL also
    { doneWaitTail = node; doneWaitHead = node; }
  else // insert to tail
    { doneWaitTail->next = node; doneWaitTail = node; }
}
void list_events (eventNode *event)
{
  if (event != NULL)
    { printf ("Event: time=%d, pid=%d, action=%d, recurP=%d, ",
	      event->time, event->pid, event->act, event->recurP);
      if (event->left != NULL) printf ("left=%d, ", event->left->time);
      else printf ("left=null, ");
      if (event->right != NULL) printf ("right=%d\n", event->right->time);
      else printf ("right=null\n");
      list_events (event->left);
      list_events (event->right);
    }
}
// move all processes in doneWait list to ready queue, empty the list
// before moving, perform necessary doneWait actions
void remove_eventhead ()
{ struct eventNode *event, *temp;

  event = eventHead;
  if (event->right != NULL)
    { temp = event->right;
      while (temp->left != NULL) temp = temp->left;
      eventHead = temp;
      event->right->parent = event->parent;
    }
  else eventHead = event->parent;
  event->parent->left = event->right;
}
void dump_events ()
{
  list_events (eventTree);
  printf ("EventHead: time=%d, pid=%d, action=%d, recurP=%d\n",
	  eventHead->time, eventHead->pid, eventHead->act, eventHead->recurP);
}
void dump_donewait_list ()
{ doneWaitNode *node;

  node = doneWaitHead;
  printf ("doneWait List = ");
  while (node != NULL) { printf ("%d, ", node->pid); node = node->next; }
  printf ("\n");
}


void doneWait_moveto_ready ()
{ doneWaitNode *temp;

  while (doneWaitHead != NULL)
    { temp = doneWaitHead;
      insert_ready (temp->pid);
      doneWaitHead = temp->next;
      free (temp);
    }
  doneWaitTail = NULL;
}
void insert_event (eventNode *event)
{ struct eventNode *cnode;

  event->left = NULL;
  event->right = NULL;

  cnode = eventTree;
  while (cnode != NULL)
    { if (event->time < cnode->time)
	if (cnode->left == NULL)
	  { cnode->left = event;
	    event->parent = cnode;
	    if (eventHead == cnode) eventHead = event;
	    // the new event has a lower time, eventHead should point to it
	    break;
	  }
	else cnode = cnode->left;
      else // event->time >= tree->time
	{ if (cnode->right == NULL)
	    { cnode->right = event;
	      event->parent = cnode;
	      break;
	    }
	  else cnode = cnode->right;
	}
    }
}


void check_timer ()
{ struct eventNode *event;

  while (eventHead->time <= CPU.numCycles)
    { event = eventHead;
      if (Debug)
	{ printf ("Process event: time=%d, pid=%d, action=%d, recurP=%d\n",
		  event->time, event->pid, event->act, event->recurP);
	  printf ("Check timer: interrupt =  %x ==> ", CPU.interruptV);
	}
      switch (event->act)
	{ case actTQinterrupt:
	    set_interrupt (tqInterrupt);
	    break;
	case actAgeInterrupt:
	  set_interrupt (ageInterrupt);
	  break;
	case actReadyInterrupt:
	  insert_doneWait_process (event->pid);
	  set_interrupt (doneWaitInterrupt);
	case actNull:
	  if (Debug) printf ("Event: time=%d, pid=%d, action=%d, recurP=%d\n",
			     event->time, event->pid, event->act, event->recurP);
	  break;
	default:
	  printf ("Encountering an illegitimate action code\n");
	  break;
	}
      remove_eventhead ();
      if (event->recurP > 0) // recurring event, put the event back
	{ event->time = CPU.numCycles + event->recurP;
	  if (event->time > maxCPUcycles)
	    { printf ("timer exceeds CPU cycle limit!!!\n"); exit(-1); }
	  else insert_event (event);
	}
      else free (event);
      if (Debug) { printf ("Interrupt: %x\n", CPU.interruptV); dump_events (); }
    }
}


void deactivate_timer (genericPtr castedevent)
{  eventNode *event,*castedevent1;
  castedevent1= (eventNode *)castedevent;
  event = castedevent1;
  event->act = actNull;
  if (Debug)
    { printf ("Deactivate event: addr=%x, time=%d, pid=%d, action=%d, recurP=%d\n",
	      castedevent, event->time, event->pid, event->act, event->recurP);
      //dump_events ();
    }
}

void dump_memory(int pid)
{
  int i,start,frame;
  char continue1;
  memoryframe *m;
  printf ("************ Instruction Memory Dump for Process %d\n", pid);

  for (i=0; i<PCB[pid].numInstr; i++)
    {
      frame=PCB[pid].Mbase+i;
      m= computeaddress(frame);
	cout<<endl;
      printf ("%d ", m->mInstr);
      printf ("\n");
      cout<<endl<<" Enter c to continue ";
      cin>>continue1;
      if(continue1!='c')
	break;
    }

  printf ("************ Data Memory Dump for Process %d\n", pid);
  for (i=0; i<PCB[pid].Mbound; i++)
    {
      frame= PCB[pid].MDbase+i;
      m= computeaddress(frame);
	cout<<endl;;
      printf ("%.2f ", m->mData);
      printf ("\n");
      cout<<endl<<" Enter c to continue ";
      cin>>continue1;
      if(continue1!='c')
        break;

    }
}

void init_pagefault_handler (int pid)
{
  int ret;
  ret=pagefaultcalculation(pid,poffset[pid]);
}
void submit_process(char *fname)
{
  FILE *fprog;
  int pid,mSize,numInstr,numData;
  int ret,i,opcode,operand;
  float data;
  fprog=fopen(fname,"r");
  if(fprog==NULL)
    {
      cout<<endl<<"In correct program name! " <<fname <<"submission failed!!! "<<endl;
      return;
    }
  noofprocess++;
  fscanf(fprog,"%d %d %d \n",&mSize,&numInstr,&numData);
  pid=get_PCB();
  allocate_spool(pid);
  PCB[pid].PC= 0;
  PCB[pid].AC= 0;
  PCB[pid].exeStatus= eReady;
  PCB[pid].numInstr= numInstr;
  PCB[pid].numData= numData;
  numi=numInstr;
  for(i=0;i<numInstr;i++)
    {
      fscanf(fprog,"%d %d \n",&opcode,&operand);
      ret=load_instruction(pid,i,opcode,operand);
    }
  for(i=0;i<numData;i++)
    {

      fscanf(fprog,"%f \n",&data);
      ret= load_data(pid,i+numInstr,data);
    }
  allocate_memory(pid,mSize,numInstr);
  if(PCB[pid].exeStatus==eReady)
    insert_ready(pid);
  else
    cout<<endl<<"process"<<pid<<"loading was unsuccessful";
}
void process_command()
{
  char action;
  char fname[100];
  int pid,time,ret;
  cout<<"command>";
  cin>>action;
  while(action!='T')
    {
      switch(action)
	{
	case 's':
	  {
	    cout<<"Filename:";
	    cin>>fname;
	    cout<<endl<<"Filename "<<fname <<" is submitted "<<endl;
	    submit_process(fname);
	    break;
	  }
	case 'x':
	  {
	    execute_process();
	    break;
	  }
	case 'r' :
	  {
	    dump_registers();
	    break;
	  }
	case 'q' :
	  {
	    dump_ready_queue();
	    dump_donewait_list();
	    break;
	  }
	case 'p':
	  {
	    cout<< endl<<"PCB Dump starts checks from 0 to :"<<currentPid;
	    for(pid=1;pid<currentPid;pid++);
	    dump_memory(pid);
	    break;
	  }
	case 'e':
	  {
	    dump_events ();
	    break;

	  }
	case 'm':   // dump Memory
	  {
	    for (pid=1; pid<currentPid; pid++)
	      if (PCB[pid].Pid!= 0)
		dump_memory (pid);
	    for (pid=1; pid<currentPid; pid++)
	      {
		if (PCB[pid].Pid != 0)
		  {
		    cout<<" The page table entries for process : "<<pid<<" are :";
		    ptg[pid].showpagetable();
		  }
	      }
	    freelistdisplay();
	    swaplistdisplay();

	    break;
	  }
	case 'l':   // dump Spool
	  {
	    for (pid=1; pid<currentPid; pid++)
	      if (PCB[pid].Pid != 0)
		dump_spool (pid);
	    break;
	  }
	case 'T':
	  {
	    break;
	  }
	}
      cout<<endl<<"command>";
      cin>>action;
    }
}

// Function containing task to be performed b IOthread

void *iothread(void *arg)
{
 r:
  ioqdata data;
  sem_wait(&IObegin);
  if(done!=1)
    {
      cout<<endl<<" IO thread processing page fault request ";
      cout<<endl;
      data= ioq.dequeue();
      {
	switch(data.task)
	  {
	  case 1:
	    {
	      data.m1->mData= data.m2->mData;
	      data.m1->mInstr= data.m2->mInstr;
	      ptg[data.pid].put(poffset[data.pid],data.fno);
	      break;
	    }
	  case 2:
	    {
	      data.m3->mData =data.m2->mData;
	      data.m3->mInstr =data.m2->mInstr;
	      data.m3->age= 0;
	      data.m2->age= 0;
	      data.m2->dirtybit= 0;
	      ptg[data.swappedpid].deletepage(data.swappedoffset);
	      ptg[data.pid].put(poffset[data.pid],data.m3->frameno);
	    }
	  case 3:
	    {
	      data.m2->age= 0;
	      data.m2->mData =0;
	      data.m2 ->mInstr=0;
	      data.swappedoffset=ptg[data.swappedpid].getoffset(data.swappedframe);
	      ptg[data.swappedpid].deletepage(data.swappedoffset);
	      ptg[data.pid].put(poffset[data.pid],data.m2->frameno);
	    }
	  }
      }
      CPU.interruptV= actReadyInterrupt;
    }
  goto r;
}
void init_idle_process ()
{
  PCB[idlePid].Pid = idlePid;
  PCB[idlePid].PC = 0;
  PCB[idlePid].AC = 0;
  PCB[idlePid].numInstr = 2;
  PCB[idlePid].numStaticData = 1;
  PCB[idlePid].numData = 1;
  if (Debug) dump_PCB (idlePid);

  // load instructions and data into memory
  load_instruction (idlePid,0, OPifgo, 0);
  load_instruction ( idlePid,1, OPifgo, 0);
  load_data ( idlePid,0, 1.0);
  allocate_memory (idlePid, idleMsize, idleNinstr);
  if (Debug) dump_memory (idlePid);
}

void initialize_process ()
{
  currentPid = 2;  // the next pid value to be used
  init_idle_process ();
}
void initialize_eventtree ()
{
  eventTree = (struct eventNode *) malloc (sizeof (struct eventNode));
  eventTree->time = maxCPUcycles + 1;
  eventTree->pid = 0;
  eventTree->act = 0;
  eventTree->recurP = 0;
  eventTree->left = NULL;
  eventTree->right = NULL;
  eventTree->parent = NULL;
  eventHead = eventTree;

}
void initialize_timer ()
{
  initialize_eventtree();
}

genericPtr add_timer (int time,int pid,int action,int recurperiod)
{ eventNode *event;
  time = CPU.numCycles + time;
  if (time > maxCPUcycles)
    { printf ("timer exceeds CPU cycle limit!!!\n"); exit(-1); }
  else
    { event = new eventNode;
      event->time = time;
      event->pid = pid;
      event->act = action;
      event->recurP = recurperiod;
      insert_event(event);
      if (Debug) printf ("Set timer: time=%d, pid=%d, action=%d, recurP=%d\n",
			 event->time, event->pid, event->act, event->recurP);
      return ((genericPtr) event);
      // to not expose the eventNode structure, a casted pointer is returned
    }
}

void initialize_system()
{
  FILE *fconfig;
  fconfig=fopen("config.sys","r");
  fscanf(fconfig,"%d \n",&Observe);
  fscanf(fconfig,"%d %d\n",&cpuQuantum,&idleQuantum);
  fscanf(fconfig,"%d %d %d %d \n",&pageSize,&memSize,&swapSize,&OSmemSize);
  fscanf(fconfig,"%d  \n",&periodAgeScan);
  fscanf(fconfig,"%d \n",&spoolPsize);
  fclose(fconfig);
  initialize_memory();
  initialize_cpu();
  initialize_process;
  initialize_timer();
  pthread_t iothread1;
  sem_init(&IObegin,0,0);
  pthread_create(&iothread1,NULL,iothread,NULL);
}
void clean_process (int pid)
{
  //free_memory (pid);
  free_spool (pid);
  free_PCB (pid);  // PCB has to be freed last, other frees use PCB info
}

void end_process (int pid)
{
  PCB[pid].exeStatus = CPU.exeStatus;
  PCB[pid].spoolPos = CPU.spoolPos;
  // PCB[pid] is not updated, no point to do a full context switch
  // but the information to be used by the spooler needs to be updated

  if (CPU.exeStatus == eError)
    { printf ("Process %d has an error, dumping its states\n", pid);
      dump_PCB (pid);
      dump_memory (pid);
      dump_spool (pid);
    }
  print_spool (pid);
  clean_process (pid);
  // if there is a real printer, printing would be slow
  // cpu should continue with executing other processes
  // and cleaning is only done after the printer acknowledges
}

void free_PCB(int pid)
{
  // free (PCB[pid]);
  if (Debug) printf ("Free PCB effect: %x\n", PCB[pid]);
  //PCB[pid] = NULL;
}
void memory_agescan()
{int i;
  for(i=0;i<memSize*8/pageSize;i++)
    {
      if(mf[i].age!=0)
	mf[i].age>>1;
    }
}

void handle_interrupt ()
{
  if ((CPU.interruptV & ageInterrupt) == ageInterrupt)
    memory_agescan ();
  if ((CPU.interruptV & doneWaitInterrupt) == doneWaitInterrupt)
    doneWait_moveto_ready ();
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
void increase_numcycles ()
{
  CPU.numCycles++;

  if (CPU.numCycles > maxCPUcycles)
    { printf ("CPU cycle count exceeds its limit!!!\n"); exit(-1); }
  // in a real system, timer is checked on clock cycles
  // here we use CPU cycle for timer, so timer check is done here
  check_timer ();
}
void cpu_execution ()
{ float sum, temp;
  int mret, spret, gotoaddr;
  char str[32];

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
		CPU.AC = CPU.MBR; break;
	    case OPadd:
	      CPU.AC = CPU.AC + CPU.MBR; break;
	    case OPsum:
	      temp = CPU.MBR;  sum = 0;
	      while (temp > 0) { sum = sum+temp; temp = temp-1; }
	      CPU.AC = CPU.AC + sum;
	      break;
	    case OPifgo:  // conditional goto, need two instruction words
	      gotoaddr = CPU.IRoperand; // this is the second operand
	      if (Debug) printf ("Process %d executing: Goto %d, If %d,%f\n",
				 CPU.Pid,  gotoaddr, CPU.IRoperand, CPU.MBR);
	      if (CPU.MBR > 0) CPU.PC = gotoaddr - 1;
	      // Note: PC will be ++, so set to 1 less
	      break;
	    case OPstore:
	      put_data (CPU.IRoperand); break;
	    case OPprint:
	      sprintf (str, "M[%d]=%.2f\n", CPU.IRoperand, CPU.MBR);
              // if the printing becomes longer, change str size
	      spret = spool (str);
	      if (spret == spError) CPU.exeStatus = eError;
	      break;
	    case OPsleep:
	      add_timer (CPU.IRoperand, CPU.Pid, actReadyInterrupt, oneTimeTimer);
	      CPU.exeStatus = eWait; break;
	    case OPend:
	      CPU.exeStatus = eEnd; break;
	    default:
	      printf ("Illegitimate OPcode in process %d\n", CPU.Pid);
	      CPU.exeStatus = eError;
	    }
	  CPU.PC++;
	}

      // no matter whether there is a page fault or an error,
      // should handle cycle increment and interrupt
      increase_numcycles ();
      if (CPU.interruptV != 0) handle_interrupt ();
    }
}

void execute_process ()
{ int pid;
  genericPtr event;

  pid = get_ready ();
  if (pid != nullReady)
    { context_in (pid);
      CPU.exeStatus = eRun;
      event = add_timer (cpuQuantum, CPU.Pid, actTQinterrupt, oneTimeTimer);
      cpu_execution ();
      if (CPU.exeStatus == eReady)
	{ context_out (pid);
	  remove_ready (); insert_ready (pid);
	}
      else if (CPU.exeStatus == ePFault)
	{ context_out (pid);
	  remove_ready (); deactivate_timer (event);
	  init_pagefault_handler (CPU.Pid);
	}
      else if (CPU.exeStatus == eWait)
	{ context_out (pid);
	  remove_ready (); deactivate_timer (event);
	}
      else if( CPU.exeStatus == eError || CPU.exeStatus == eEnd)
	{ remove_ready (); end_process (pid);
	  deactivate_timer (event); }
    }
  // ePFault and eWait has to be handled differently
  // in ePFfault, memory has to handle the event
  // in eWait, CPU directly execute IO libraries and initiate IO
  //
  // In case of eWait/ePFault/eError/eEnd, timer should be deactivated,
  // otherwise, it has the potential of impacting the next round of exe
  // but if time quantum just expires when the above cases happends,
  // event would have just been freed, our deactivation can be dangerous

  else // no ready process in the system
    { context_in (idlePid);
      CPU.exeStatus = eRun;
      add_timer (idleQuantum, CPU.Pid, actTQinterrupt, oneTimeTimer);
      cpu_execution ();
    }
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
  printf ("spoolPos = %d\n", CPU.spoolPos);
  printf ("exeStatus = %d\n", CPU.exeStatus);
  printf ("InterruptV = %x\n", CPU.interruptV);
  printf ("numCycles = %d\n", CPU.numCycles);
}

void dump_ready_queue ()
{ int i, next;

  printf ("******************** Ready Queue Dump\n");
  printf ("readyCount = %d; readyHead = %d; readyTail = %d\n",
	  readyCount, readyHead, readyTail);
  for (i=0; i<readyCount; i++)
    { next = (readyHead+i) % maxReady;
      printf ("readyList[%d] = %d\n", next, readyList[next]);
    }
}
void dump_spool (int pid)
{ int last;
  int i;

  last = PCB[pid].spoolPos;
  printf ("********** Spool Dump for Process %d with size %d\n", pid, last);
  for (i=0; i<last; i++)
    if (PCB[pid].spoolPtr[i] < 32 || PCB[pid].spoolPtr[i] > 126)
      // printable characters are between 32 and 126
      printf ("\\%d\\", PCB[pid].spoolPtr[i]);
    else printf ("%c", PCB[pid].spoolPtr[i]);
  printf ("\n");
}

int spool (char *str)
{ int len;

  len = strlen (str);
  if (Debug) printf ("Spool string: %s; len = %d\n", str, len);
  if (CPU.spoolPtr == NULL)
    { printf ("Spool for Process %d does not exist!!!\n", CPU.Pid);
      return (spError);
    }
  else if (CPU.spoolPos+len > spoolPsize)
    { printf ("Spool space overflow: Process %d Length %d!!!\n", CPU.Pid, len);
      return (spError);
    }
  else { strncpy (CPU.spoolPtr+CPU.spoolPos, str, len);
    CPU.spoolPos = CPU.spoolPos + len;
    return (spNormal);
  }
}

void allocate_spool (int pid)
{
  PCB[pid].spoolPtr = (char*)spoolPsize;
  PCB[pid].spoolPos = 0;
}

void free_spool (int pid)
{
  //free (PCB[pid].spoolPtr);
  if (Debug) printf ("Free spoolPtr effect: %x\n", PCB[pid].spoolPtr);
  PCB[pid].spoolPtr = NULL;
}

// printer call should be a send, send the print requst to the printer
// should select a free printer to send the print job to
// if one printer has an error return, send to another printer
//
void print_spool (int pid)
{ int ret;

  ret = printer (pid, PCB[pid].exeStatus,PCB[pid].spoolPtr, PCB[pid].spoolPos);

}
int printer (int pid, int status, char *prstr, int  len)
{ int i;

  printf ("\n======== Printout for Process %d ========\n", pid);
  if (status == eError)
    printf ("Process %d had encountered error in execution!!!\n", pid);
  else  // was eEnd
    printf ("Process %d had completed successfully!!!\n", pid);
  printf ("=========================================\n\n");

  for (i=0; i<len; i++) printf ("%c", prstr[i]);
  printf ("=========================================\n\n");

  return (prNormal);
  // should simulate failure situations: out of paper, toner low, etc.
}
void initialize_cpu ()
{
  CPU.interruptV= 0;
  CPU.numCycles= 0;
}
void context_in (int pid)
{ CPU.Pid = pid;
  CPU.PC = PCB[pid].PC;
  CPU.AC = PCB[pid].AC;
  CPU.Mbase = PCB[pid].Mbase;
  CPU.MDbase = PCB[pid].MDbase;
  CPU.Mbound = PCB[pid].Mbound;
  CPU.spoolPtr = PCB[pid].spoolPtr;
  CPU.spoolPos = PCB[pid].spoolPos;
  CPU.exeStatus = PCB[pid].exeStatus;
}

void context_out ( int pid)
{ PCB[pid].PC = CPU.PC;
  PCB[pid].AC = CPU.AC;
  PCB[pid].spoolPos = CPU.spoolPos;
  PCB[pid].exeStatus = CPU.exeStatus;
}
void dump_PCB (int pid)
{
  printf ("******************** PCB Dump for Process %d\n", pid);
  printf ("Pid = %d\n", PCB[pid].Pid);
  printf ("PC = %d\n", PCB[pid].PC);
  printf ("AC = %.2f\n", PCB[pid].AC);
  printf ("Mbase = %d\n", PCB[pid].Mbase);
  printf ("MDbase = %d\n", PCB[pid].MDbase);
  printf ("Mbound = %d\n", PCB[pid].Mbound);
  printf ("spoolPtr = %x\n", PCB[pid].spoolPtr);
  printf ("spoolPos = %d\n", PCB[pid].spoolPos);
  printf ("exeStatus = %d\n", PCB[pid].exeStatus);
  printf ("numInstr = %d\n", PCB[pid].numInstr);
  printf ("numData = %d\n", PCB[pid].numData);
  printf ("numStaticData = %d\n", PCB[pid].numStaticData);
}


// main function
int main()
{
  initialize_system();
  process_command();
  done= 1;


  return(0);



}

// End of the project code