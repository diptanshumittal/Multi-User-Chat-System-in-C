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

int counter = 0;
struct cDetails {
	int index;
	int sockID;
	struct sockaddr_in cAddr;
	int len;
};
struct Node{
	struct cDetails Client;
	struct Node* next;
};
struct Node *clients = NULL;
pthread_t thread[1024];


void * clientThread(void * tclient){
	struct cDetails* ClientDetail = (struct cDetails*) tclient;
	int index = ClientDetail->index, cSoc = ClientDetail->sockID;
	printf("Client number %d connected.\n",index + 1);
	while(1){
		char data[1024];
		int read ;
		if(read = recv(cSoc,data,1024,0)>-1){
			char output[1024];
			if(strcmp(data,"DATASET") == 0){
				int l = 0;
				struct Node* temp = clients ;
				int y=0;
				while(y<counter){
					if(index!=temp->Client.index)
						l += snprintf(output + l,1024,"Client %d at socket %d.\n",temp->Client.index + 1,temp->Client.sockID);
					y++;
					temp = temp->next;
				}
				send(cSoc,output,1024,0);
				continue;
			}
			if(strcmp(data,"SEND") == 0){
				int l ;		
				int flag=0;
				read = recv(cSoc,data,1024,0);
				data[read] = '\0';
				if(strcmp(data,"*")==0){
					read = recv(cSoc,data,1024,0);
					data[read] = '\0';
					struct Node* temp = clients ;
					for(int i = 0 ; i < counter ; i ++){
						if(temp->Client.index != index){
							l=0;
							char we[50];
							l +=snprintf(we + l,1024,"Client %d at socket %d : \n",index+1,cSoc);
							printf("%s\n",we );
							send(temp->Client.sockID,we,1024,0);
							send(temp->Client.sockID,data,1024,0);
							flag=1;
						}
						if(temp->next!=NULL)
							temp = temp->next;
						else
							i=counter;
					}
				}
				else{
					int id = atoi(data) - 1;
					read = recv(cSoc,data,1024,0);
					data[read] = '\0';
					int y = 0;
					struct Node* temp = clients ;
					while(y<counter){
						if(temp->Client.index == id){
							send( temp->Client.sockID , data , 1024 , 0);
							flag=1;
							break;
						}
						if(temp->next!=NULL)
							temp = temp->next;
						else 
							y=counter;
					}				
				}
				if(flag==0)
					send( cSoc , "Client not available or out of range\nUse the DATASET to check the availability" , 1024 , 0);
				else
					send( cSoc , "Messages sent successfully" , 1024 , 0);				
				continue;
			}
			if(strcmp(data,"EXIT")==0){
				printf("Exited and database cleared\n");
				struct Node* temp = clients ;
				struct Node* temp1 = clients ;
				for(int i = 0 ; i < counter ; i ++){
					if(temp->Client.index == index){
						if(i==0)
							clients = clients->next;
						else{
							for(int j=0 ; j<i ; j++)
								temp1 = temp1->next;	
							temp1->next = temp->next;
						}					
					}
					temp=temp->next ;
				}
				counter-=1;
				break;
			}
		}
	}
	return NULL;
}

int main(){
	int sSoc = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	server.sin_addr.s_addr = htons(INADDR_ANY);
	if(bind(sSoc,(struct sockaddr *) &server , sizeof(server)) == -1) 
		return 0;
	if(listen(sSoc,1024) == -1) 
		return 0;
	printf("Server started\n");
	struct Node *temp = NULL;
	clients = (struct Node*)malloc(sizeof(struct Node)) ;
	struct cDetails Client ;
	temp = clients ;
	int clientCounter = counter ;
	while(1){
		Client.sockID = accept(sSoc, (struct sockaddr*) &Client.cAddr, &Client.len);
		Client.index = clientCounter;
		pthread_create(&thread[clientCounter], NULL, clientThread, (void *) &Client);
		temp->Client = Client ;
		temp->next = (struct Node*)malloc(sizeof(struct Node)) ;
		temp = temp->next;
		counter ++;
		clientCounter ++ ;
	}
}