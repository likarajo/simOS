#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "computer.h"

// sum the numbers in the input file
int compute(char *file)
{ FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int result = 0, sum = 0, number;
  int m = -1;

  fp = fopen(file, "r");
  if (fp == NULL) return 0;

  while (m != 0 && (read = getline(&line, &len, fp)) != -1)
  { number = atoi(line);
    if (m == -1) m = number;
    else { sum = sum + number; m--; }
  }
  fclose(fp);
  if (line) free(line);
  return sum;
}

void execute(int interval)
{ request_t *req;
  int result, ret;

  while ((req = dequeue()) != NULL)
  { result = compute(req->filename);
    send_client_result (req, result);
    usleep(interval);
  }
}

void main(int argc, char *argv[])
{ char cmd[80];
  char buffer[256];
  int interval;

  if (argc < 3)
  { fprintf(stderr, "ERROR, usage Admin.exe server-port-number S\n");
    exit(1);
  }
  start_client_reqhandler(argv[1]);
  interval = atoi(argv[2]);
  printf("Admin-Computer, %d\n", getpid());

  Active = 1;
  while (Active)
  { bzero(cmd, sizeof(cmd));
    printf("Please enter a command: ");
    scanf("%s", cmd);

    printf("Command: %c\n", cmd[0]);
    if (cmd[0] == 'T' || cmd[0] == 't') Active = 0; 
    else if (cmd[0] == 'Q' || cmd[0] == 'q') dump_queue();
    else if (cmd[0] == 'X' || cmd[0] == 'x') execute(interval);
  }
  end_client_reqhandler();
}
