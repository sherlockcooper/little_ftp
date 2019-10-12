#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <string.h>

#define MAXBUF 1024*1024

int int_to_str(int num, char *buf)
{
  int len=0;
  char temp[MAXBUF];
  while(num>=10)
  {
    temp[len]=(num%10)+'0';
    num/=10;
    len++;
  }
  temp[len]=num+'0';
  len++;
  for(int i=0; i<len; i++)
  {
    buf[i]=temp[len-1-i];
  }
  buf[len]=0;
  return len;
}

void uppercase(char *p) {
  for ( ; *p; ++p) *p = toupper(*p);
}

void echo(int sd) {
    char bufin[MAXBUF];
    struct sockaddr_in remote;
    char ret_buf[MAXBUF]={0};
    int seq_num=1;

    /* need to know how big address struct is, len must be set before the
       call to recvfrom!!! */
    socklen_t len = sizeof(remote);

    while (1) {
      /* read a datagram from the socket (put result in bufin) */
      int n = recvfrom(sd, bufin, MAXBUF, 0, (struct sockaddr *) &remote, &len);

      if (n < 0) {
        perror("Error receiving data");
      } 
      else {
        uppercase(bufin);
        int head_len=int_to_str(seq_num, ret_buf);
        ret_buf[head_len]=' ';
        for(int i=0; i<n; i++)
        {
          ret_buf[head_len+1+i]=bufin[i];
        }
        ret_buf[head_len+1+n]=0;
        /* Got something, just send it back */
        //sendto(sd, bufin, n, 0, (struct sockaddr *)&remote, len);
        printf("get: %s\n", bufin);
        printf("sendback: %s\n", ret_buf);
        ret_buf[strlen(ret_buf)]='\n';
        //sendto(sd, ret_buf, n+head_len+1, 0, (struct sockaddr *)&remote, len);
        sendto(sd, ret_buf, strlen(ret_buf), 0, (struct sockaddr *)&remote, len);
        seq_num++;
      }
    }
}

/* server main routine */

int main() {
  int ld;
  struct sockaddr_in skaddr;
  socklen_t length;

  /* create a socket
     IP protocol family (PF_INET)
     UDP protocol (SOCK_DGRAM)
  */

  if ((ld = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("Problem creating socket\n");
    exit(1);
  }

  /* establish our address
     address family is AF_INET
     our IP address is INADDR_ANY (any of our IP addresses)
     the port number is 9876
  */

  skaddr.sin_family = AF_INET;
  skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  skaddr.sin_port = htons(8080);

  if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr)) < 0) {
    printf("Problem binding\n");
    exit(0);
  }

  /* find out what port we were assigned and print it out */

  length = sizeof(skaddr);
  if (getsockname(ld, (struct sockaddr *) &skaddr, &length) < 0) {
    printf("Error getsockname\n");
    exit(1);
  }

  /* Go echo every datagram we get */
  echo(ld);
  return 0;
}
