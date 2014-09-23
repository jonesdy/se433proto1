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

typedef struct{
	int s;
	char c;
} X;

//globals
int recurrence;
struct sockaddr_in bindto;
time_t starttime;

//FIXME:check for timeout and handle it
void * listenandprint(void *s){
	int *sock = s;
	while(1){
		char data[1000]={0};
		
		int numbytes = recv(*sock, data, 999, 0);
		if(numbytes == 0 ||time(NULL)-starttime> 20)
			return NULL;
		X *rcvd = (X *)data;
		rcvd->s = ntohl(rcvd->s);/*unpack endianness*/
		printf("\nI am RX and I got a ");
		printf("%d\n",rcvd->s);
	}
}

void * sendstuff(void *s){
	sleep(5);
	int sock = *(int*)s;
	X tosend = {0};
	while(tosend.s < 20){
		tosend.s ++;
		tosend.s = htonl(tosend.s);
		sendto(sock, &tosend, sizeof(tosend), 0, (const struct sockaddr *)&bindto, sizeof(bindto));
		tosend.s = ntohl(tosend.s);
		printf("I am TX and I am going to send a %d\n", tosend.s);
		usleep(recurrence*1000);

	}
	printf("a nice done message\n");
	sleep(10);
	return NULL;

}




int main(int argc, char *argv[]){
	starttime = time(NULL);
	if(argc < 6){
		printf("usage: %s destip (listenip|all) destport listenport txrate\n", argv[0]);
		return;
	}
	recurrence = atoi(argv[5]);
	printf("delay is %d\n", recurrence);



	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){printf("error initializing socket %d", sock);return;}

	//struct sockaddr_in bindto;
	bindto.sin_family = AF_INET;
	bindto.sin_port = htons(atoi(argv[4]));
	if(strcmp(argv[2], "all")==0)
	{
		printf("binding to all interfaces\n");
		bindto.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");/*htonl((127 << 24)+1);//localhost*/
	}
	else
		bindto.sin_addr.s_addr = inet_addr(argv[2]);//htonl(INADDR_ANY);//inet_addr("127.0.0.1");/*htonl((127 << 24)+1);//localhost*/
	if(bind(sock, (const struct sockaddr *)&bindto, sizeof(bindto)) == 0)
		printf("listening on %d\n", atoi(argv[4]));
	else
		printf("failed to bind %d (is it in use?)\n", atoi(argv[2]));
	/*socket is bound so we can now change these to the destination address and socket*/
	bindto.sin_addr.s_addr = inet_addr(argv[1]);
	bindto.sin_port = htons(atoi(argv[3]));

	struct timeval tv={0};
	tv.tv_sec = 3;
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


