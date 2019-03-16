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

void main(int argc, char *argv[]) {
  int sockfd, portno, ret;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256], cin[256];
  char *client_id, *result, *token = NULL;

  if (argc < 4)
  { fprintf(stderr, "Usage: Client-ID Server-host-name Server-port-number\n");
    exit(0);
  }

  portno = atoi(argv[3]);
  client_id = argv[1];
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");

  server = gethostbyname(argv[2]);
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
    sprintf(buffer, "%s %s", client_id, cin);
    
    ret = send(sockfd, buffer, strlen(buffer), 0);
    if (ret < 0) error("ERROR writing to socket");

    bzero(buffer, sizeof(buffer));
    ret = recv(sockfd, buffer, sizeof(buffer), 0);
    if (ret == 0) break;

    token = strtok(buffer, " ");
    token = strtok(NULL, " ");
    result = (char *)malloc(strlen(token));
    strcpy(result, token);

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "%s, %s, %s", client_id, cin, result);

    if (ret < 0) error("Client ERROR reading from socket");
    printf("%s\n", buffer);
  }
}
