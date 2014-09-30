/*sockets*/
#include <sys/socket.h>

/*dns*/
#include <netdb.h>

/*getopt*/
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*threads*/
#include <pthread.h>

/*time*/
#include <time.h>

/*timeout sets errno*/
#include <errno.h>

#define STARTUPTIME 5
#define RUNTIME 20
#define LISTENTIME 10

#define NUMTOSEND 20

#define ARG_DESTIP 1
#define ARG_SOURCEIP 2
#define ARG_DESTPORT 3
#define ARG_SOURCEPORT 4
#define ARG_DELAY 5

#define USPERMS 1000

#define LISTENTIMEOUT 3

//globals
int recurrence;
struct sockaddr_in bindto;
time_t starttime;

void *listenandprint(void *s)
{
   int *sock = (int*)s;
   while(1)
   {
      if(time(NULL)-starttime > RUNTIME)
      {
         printf("listening thread quitting\n");
         return NULL;
      }
      char data[sizeof(int32_t)] = {0};
      
      //struct timespec timeout = {0};//use this if using pselect
      struct timeval timeout = {0};//use this if using select
      timeout.tv_sec = LISTENTIMEOUT;

      fd_set waiton;
      FD_ZERO(&waiton);
      FD_SET(*sock, &waiton);
      //pselect(*sock+1/*what the hell does this argument even do*/, &waiton, NULL, NULL,&timeout, NULL);
      select(*sock+1/*what the hell does this argument even do*/, &waiton, NULL, NULL,&timeout);
      if(FD_ISSET(*sock, &waiton) == 0)
      {
         printf("receive timed out\n");
         continue;
      }
      int numbytes = recv(*sock, data, sizeof(int32_t), 0);
      int32_t *rcvd = (int32_t *)data;
      *rcvd = ntohl(*rcvd);/*unpack endianness*/
      printf("\x1b[31mI am RX and I got a %d\n\x1b[0m", *rcvd);
   }
}

void *sendstuff(void *s)
{
   sleep(STARTUPTIME);
   int sock = *(int*)s;
   int32_t tosend = 0;
   while(tosend < NUMTOSEND)
   {
      tosend ++;
      printf("\x1b[32mI am TX and I am going to send a %d\n\x1b[0m", tosend);
      tosend = htonl(tosend);
      sendto(sock, &tosend, sizeof(tosend), 0, (const struct sockaddr *)&bindto, sizeof(bindto));
      tosend = ntohl(tosend);
      usleep(recurrence*USPERMS);
   }
   printf("TX is done\n");
   sleep(LISTENTIME);
   return NULL;
}

int main(int argc, char *argv[])
{
   starttime = time(NULL);
   if(argc < ARG_DELAY+1)
   {
      printf("usage: %s destip (listenip|all) destport listenport txrate\n", argv[0]);
      return 0;
   }
   recurrence = atoi(argv[ARG_DELAY]);
   printf("delay is %d\n", recurrence);

   int sock = socket(AF_INET, SOCK_DGRAM, 0);
   if(sock < 0)
   {
      printf("error initializing socket %d", sock);
      return 1;
   }

   bindto.sin_family = AF_INET;
   bindto.sin_port = htons(atoi(argv[ARG_SOURCEPORT]));
   if(strcmp(argv[ARG_SOURCEIP], "all")==0)
   {
      printf("binding to all interfaces\n");
      bindto.sin_addr.s_addr = htonl(INADDR_ANY);
   }
   else
   {
      bindto.sin_addr.s_addr = inet_addr(argv[ARG_SOURCEIP]);
   }
   if(bind(sock, (const struct sockaddr *)&bindto, sizeof(bindto)) == 0)
   {
      printf("listening on %d\n", atoi(argv[ARG_SOURCEPORT]));
   }
   else
   {
      printf("failed to bind %d (is it in use?)\n", atoi(argv[ARG_SOURCEPORT]));
   }
   /*socket is bound so we can now change these to the destination address and socket*/
   bindto.sin_addr.s_addr = inet_addr(argv[ARG_DESTIP]);
   bindto.sin_port = htons(atoi(argv[ARG_DESTPORT]));

   //struct timeval tv={0};
   //tv.tv_sec = LISTENTIMEOUT;
   //setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

   pthread_t listenthread;
   pthread_t sendthread;

   pthread_create(&listenthread, NULL, &listenandprint, (void*)&sock);
   pthread_create(&sendthread, NULL, &sendstuff, (void*)&sock);

   pthread_join(listenthread, NULL);
   pthread_join(sendthread, NULL);
   printf("closing socket\n");
   close(sock);
}
