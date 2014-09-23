/*sockets*/
#include <sys/socket.h>
/*dns*/
#include <netdb.h>

/*getopt*/
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*readline*/
#ifdef USEREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif


/*threads*/
#include <pthread.h>

FILE *teeto = NULL;

void * listenandprint(void *s){
	int *sock = s;
	while(1){
		char data[1000]={0};
		
		int numbytes = recv(*sock, data, 999, 0);
		if(teeto != NULL){
			fwrite(data, 1, numbytes, teeto);
			fwrite("\n", 1, 1, teeto);
		}
		printf("\nUDP packet:");
		printf("%s\n",data);
	}
}




int main(int argc, char *argv[]){
	short listen = 0;
	int option;
	if(argc < 2 || strcmp(argv[1], "--help")==0)goto helpmsg;
	while((option=getopt(argc, argv, "hvl:t:")) != -1){
		switch(option){
			case 'h':
			helpmsg:
				printf("Copyright 2014 Thomas VanSelus\n");
				printf("   /|\n");
				printf("  //| .^^.  \n");
				printf("  ||| (oo)   \n");
			       printf("  \\ |/  /|\\  \n");
			       printf("   \\___---.    \n");
				printf("%s %s",  argv[0], "IP PORT\n");
				printf("%s %s",  argv[0], "-l LPORT IP SPORT\n");
				printf("%s %s",  argv[0], "-v\n");
				printf("%s %s",  argv[0], "-h\n");
				printf("===========example========\n");
				printf("run\n");
				printf("%s %s",  argv[0], "-t logfile1 -l 8000 127.0.0.1 8001\n");
				printf("and\n");
				printf("%s %s",  argv[0], "-t logfile2 -l 8001 127.0.0.1 8000\n");
				printf("on the same machine for local duplex communication\n");
				printf("received communication will be logged to logfile1 and logfile2\n");
				printf("===========arguments========\n");
				printf("-h  print this help\n");
				printf("-v  print version\n");
				printf("-l LPORT listen on port number\n");
				printf("-t TEEFILE write incoming packets to file also\n");
				printf("LPORT the port to listen on\n");
				printf("SPORT the port to send to\n");
				printf("TEEFILE the file to write to\n");
#ifdef USEREADLINE
				printf("This copy of nutcat was built with readline support\n");
#else
				printf("This copy of nutcat was built without readline support\n");
#endif
				return;
			case 'v':
				printf("version 0.1\n");
				return;
			case 'l':
				listen = atoi(optarg);
				break;
			case 't':
				teeto = fopen(optarg, "a");
				break;
		}
	}
	if(optind >= argc){printf("requires port (-h for help)\n");return;}
	if(argc < 3 && !listen){printf("requires ip address (-h for help)\n");return;}
















	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){printf("error %d", sock);return;}

	struct sockaddr_in bindto;
	bindto.sin_family = AF_INET;
	if(listen){
		bindto.sin_port = htons(listen);
		bindto.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");/*htonl((127 << 24)+1);//localhost*/
		if(bind(sock, (const struct sockaddr *)&bindto, sizeof(bindto)) == 0)
			printf("listening on %d\n", listen);
		else
			printf("failed to bind %d (is it in use?)\n", listen);
	}
	/*socket is bound so we can now change these to the destination address and socket*/
	bindto.sin_addr.s_addr = inet_addr(argv[argc-2]);
	bindto.sin_port = htons(atoi(argv[argc-1]));



	pthread_t listenthread;

	if(listen){
		pthread_create(&listenthread, NULL, &listenandprint, (void*)&sock);
	}

	while(1){
		char *line;
#ifdef USEREADLINE
		line = readline("nutcat:");
		if(!line){break;}
		add_history(line);
#else
		printf("nutcat:");
		line = malloc(1000);
		scanf("%999s", line);
#endif

		sendto(sock, line, strlen(line), 0, (const struct sockaddr *)&bindto, sizeof(bindto));

		free(line);

		

	}
	printf("closing socket\n");
	close(sock);
	if(teeto)
		fclose(teeto);
	

}


