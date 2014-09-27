/*sockets*/
#include <sys/socket.h>

/*dns*/
#include <netdb.h>

/*getopt*/
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct{
	int s;
//	char c;
} X;

//globals
int recurrence;
struct sockaddr_in bindto;
time_t starttime;

void * listenandprint(void *s){
	int *sock = (int*)s;
	while(1){
		if(time(NULL)-starttime > RUNTIME)
			return NULL;
		char data[sizeof(X)]={0};
		
		int numbytes = recv(*sock, data, sizeof(X), 0);
		if(errno == EAGAIN|| errno == EWOULDBLOCK ||numbytes == 0)
		{
			errno = 0;/*clear error flag so we don't keep continuing*/
			continue;
		}
		X *rcvd = (X *)data;
		rcvd->s = ntohl(rcvd->s);/*unpack endianness*/
		printf("\nI am RX and I got a ");
		printf("%d\n",rcvd->s);
	}
}

void * sendstuff(void *s){
	sleep(STARTUPTIME);
	int sock = *(int*)s;
	X tosend = {0};
	while(tosend.s < NUMTOSEND){
		tosend.s ++;
		tosend.s = htonl(tosend.s);
		sendto(sock, &tosend, sizeof(tosend), 0, (const struct sockaddr *)&bindto, sizeof(bindto));
		tosend.s = ntohl(tosend.s);
		printf("I am TX and I am going to send a %d\n", tosend.s);
		usleep(recurrence*USPERMS);

	}
	printf("a nice done message\n");
	sleep(LISTENTIME);
	return NULL;

}




int main(int argc, char *argv[]){
	starttime = time(NULL);
	if(argc < ARG_DELAY+1){
		printf("usage: %s destip (listenip|all) destport listenport txrate\n", argv[0]);
		return 0;
	}
	recurrence = atoi(argv[ARG_DELAY]);
	printf("delay is %d\n", recurrence);



	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){printf("error initializing socket %d", sock);return 1;}

	bindto.sin_family = AF_INET;
	bindto.sin_port = htons(atoi(argv[ARG_SOURCEPORT]));
	if(strcmp(argv[ARG_SOURCEIP], "all")==0)
	{
		printf("binding to all interfaces\n");
		bindto.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
		bindto.sin_addr.s_addr = inet_addr(argv[ARG_SOURCEIP]);
	if(bind(sock, (const struct sockaddr *)&bindto, sizeof(bindto)) == 0)
		printf("listening on %d\n", atoi(argv[ARG_SOURCEPORT]));
	else
		printf("failed to bind %d (is it in use?)\n", atoi(argv[ARG_SOURCEPORT]));
	/*socket is bound so we can now change these to the destination address and socket*/
	bindto.sin_addr.s_addr = inet_addr(argv[ARG_DESTIP]);
	bindto.sin_port = htons(atoi(argv[ARG_DESTPORT]));

	struct timeval tv={0};
	tv.tv_sec = LISTENTIMEOUT;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

	pthread_t listenthread;
	pthread_t sendthread;

	pthread_create(&listenthread, NULL, &listenandprint, (void*)&sock);
	pthread_create(&sendthread, NULL, &sendstuff, (void*)&sock);

	pthread_join(listenthread, NULL);
	pthread_join(sendthread, NULL);
	printf("closing socket\n");
	close(sock);
}


