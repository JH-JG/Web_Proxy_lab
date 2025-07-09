#include <stdio.h>
#include <string.h>
#include "csapp.h"
#include <pthread.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";


void doit(int client_fd);
void do_request(int proxy_fd, char *method, char *uri_to_serv, char *host);
void do_response(int fd, int proxy_fd);
int parse_uri(char *uri, char *uri_ptos, char *host, char *port);
void *conn_thread(void *arg);

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t p;

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    int *connfdp = malloc(sizeof(int));
    *connfdp = connfd; // 값을 주소로 전달, 직접 캐스팅하면 같은 것을 가리키게된다.
    Pthread_create(&p, NULL, conn_thread, connfdp); // 스레드 생성
    Pthread_detach(p); // 스레드 분리

  }
  return 0;
}

void *conn_thread(void *arg)
{
  int connfd = *((int *)arg);
  free(arg);
  doit(connfd);
  Close(connfd);

  return;
}

void doit(int client_fd)
{
  /*
  클라이언트로 부터 들어온 요청을 확인하고 실제 서버로 전송한다.
  */
  int proxy_fd;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], host[MAXLINE], port[MAXLINE];
  char uri_to_serv[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, client_fd);
  int n = Rio_readlineb(&rio, buf, MAXLINE);
  if (n <= 0)
  {
    printf("End with Error %d\n", n);
    return;
  }

  printf("Request headers:\n");
  printf("%s", buf);

  sscanf(buf, "%s %s %s", method, uri, version);

  parse_uri(uri, uri_to_serv, host, port);

  proxy_fd = Open_clientfd(host, port);
  do_request(proxy_fd, method, uri_to_serv, host);
  do_response(client_fd, proxy_fd);
  Close(proxy_fd);
}


void do_request(int proxy_fd, char *method, char *uri_to_serv, char *host)
{
  char buf[MAXLINE];
  // 첫 번째 요청 라인은 sprintf로 생성 -> sprintf buf 맨처음 부터 덮어씌움 strcat으로 이어 붙히자.
  sprintf(buf, "%s %s HTTP/1.0\r\n", method, uri_to_serv);

  // 나머지 헤더는 strcat으로 이어 붙임
  strcat(buf, "Host: ");
  strcat(buf, host);
  strcat(buf, "\r\n");
  strcat(buf, user_agent_hdr);
  strcat(buf, "Connection: close\r\n");
  strcat(buf, "Proxy-Connection: close\r\n\r\n");

  Rio_writen(proxy_fd, buf, strlen(buf));
}


void do_response(int client_fd, int proxy_fd)
{
  char buf[MAXLINE];
  int n;
  rio_t rio;

  Rio_readinitb(&rio, proxy_fd);
  // n = Rio_readlineb(&rio, buf, MAXLINE); // readlineb는 한줄만 읽어온다. 즉, 서버 응답만 읽어옴 예) HTTP 200 OK 나머지 버려짐
  while((n = Rio_readnb(&rio, buf, MAXLINE)) > 0){ // EOF가 도달할때 까지 서버가 보낸 모든 데이터를 읽어온다.
    Rio_writen(client_fd, buf, n);
  }
}


int parse_uri(char *uri, char *uri_to_serv, char *host, char *port)
{
  char *host_start;
  char *port_start;
  char *path_start;

  if (!(host_start = strstr(uri, "://"))) return -1;

  host_start += 3; // http://www.google.com/~ -> ://이후 시작점 www.google.com/~

  // path parsing
  path_start = strchr(host_start, '/'); 
  if (path_start){
    // path가 존재하면 uri_to_serv에 복사
    strcpy(uri_to_serv, path_start);
    *path_start = '\0'; // 현재 가리키는 / 를 NULL로 바꿔줘야됨(str 읽어올때 널까지 읽음) -> host 시작점이 주소만 가져옴
  }
  else{
    strcpy(uri_to_serv, '/'); // 기본 경로
  }

  // port parsing
  port_start = strchr(host_start, ':');
  if (port_start){
    strcpy(port, port_start + 1);
    *port_start = '\0';
  }
  else{
    strcpy(port, "80");
  }

  strcpy(host, host_start);

  return 0;
}