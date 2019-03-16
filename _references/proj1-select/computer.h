int Active;
    // control whether the system should remain active or terminate

// queue definitions

typedef struct request
{ int sockfd;
  char* client_id;
  char* filename;
  int port;
} request_t;

typedef struct node
{ request_t request;
  struct node *next;
} node_t;

// from queue.c
void enqueue(request_t req);
request_t* dequeue();


// from computer.c
void start_computer(char *port);
void end_computer();


