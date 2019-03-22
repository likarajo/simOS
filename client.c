#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"

void error(char *msg)
{ perror(msg);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockfd, portno, ret;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256], cin[256];
  char *client_id, *result, *token = NULL;
  char pid;

  if (argc < 2)
  { fprintf(stderr, "Usage: Server-host-name Server-port-number\n");
    exit(0);
  }

  portno = atoi(argv[2]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");

  server = gethostbyname(argv[1]);
  if (server == NULL) error("ERROR, no such host");
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  while (1)
  { bzero(buffer, sizeof(buffer));
    bzero(cin, sizeof(cin));
    result = NULL;
    token = NULL;

    printf("%s", "Please enter filename: ");
    scanf("%s", cin);
    sprintf(buffer, "%s", cin);

    ret = send(sockfd, buffer, strlen(buffer), 0);
    if (ret < 0) error("ERROR writing to socket\n");

    bzero(buffer, sizeof(buffer));
    ret = recv(sockfd, buffer, sizeof(buffer), 0);
    if(ret < 0) error("ERROR reading from socket\n");

    if (strncmp(buffer, "End Proc", 256)==0) {
        printf("Disconnected\n");
        exit(0);
    }

    printf("%s\n", buffer);

    /*token = strtok(buffer, " ");
    pid = (char *)malloc(strlen(token));
    strcpy(pid, token);
    token = strtok(NULL, " ");
    result = (char *)malloc(strlen(token));
    strcpy(result, token);

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "%s %s", pid, result);*/

  }

  return 0;
}
