#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>


int flag = 0 ;

void * tWrite(void * sockID){
	int cSoc = *((int *) sockID);
	while(1){
		char input[1024];
		scanf("%s",input);
		if(strcmp(input,"DATASET") == 0){
			send(cSoc,input,1024,0);
		}
		else if(strcmp(input,"SEND") == 0){
			send(cSoc,input,1024,0);
			scanf("%s",input);
			send(cSoc,input,1024,0);
			scanf("%[^\n]s",input);
			send(cSoc,input,1024,0);
		}
		else if(strcmp(input,"EXIT")==0){
			printf("Exit detected \n");
			send(cSoc,input,1024,0);
			close(cSoc);
			flag=1;
			break;
		}
		else{
			printf("Wrong Command entered\n");
		}	
	}
}

int main(){
	int cSoc = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(connect(cSoc, (struct sockaddr*) &server, sizeof(server)) == -1) 
		return 0;
	pthread_t thread;
	printf("Connected\n");
	pthread_create(&thread, NULL, tWrite, (void *) &cSoc );
	char data[1024];
	int read ;
	while(1){
		if(flag==1)
			break;
		if(read = recv(cSoc,data,1024,0)>0){
			printf("%s\n",data);
		}
	}
}
