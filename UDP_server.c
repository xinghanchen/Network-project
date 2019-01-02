#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>

int getEvent(char buffer[]){
	if(buffer[16] == 'l' && buffer[19] == 'i'){
		return 0;
	}
	else if(buffer[0] == 's' && buffer[16] == 'S'){
		return 1;
	}
	else if(buffer[0] == 'c' && buffer[9] == 'c') {
		return 2;
	}
	else if(buffer[0] == 's' && buffer[16] == 'T') {
		return 3;
	}
	else if(buffer[16] == 'l' && buffer[19] == 'o') {
		return 4;
	}
	else if(buffer[0] == 's' && buffer[16] == 'E') {
		return 5;
	}
	//If not match, no event.
	return -1;
}

//This holds infos on a session
struct session {
	char client_id[7]; // client are in format of client#(client1, client2)
	struct sockaddr_in client_addr; //IP address and port of the client
									//for receiving messages from server.
	time_t last_time;// Last time when server receives a message from this client.
	char token[6];	// the token of this session
	int state ;  // The state of this session.

};
typedef struct session Session;


int main() {

    int ret;
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    char recv_buffer[1024];
    int recv_len;
    socklen_t len;

    //You may need a map to hold all the sessions to find a session givne a ID.
    //I use an array just for demonstration

    struct session session_array[16];

    //This current session is a variable temporarily hold the session upon an event.
    struct session *current_session;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    //get the password database.
	FILE* password;
	password = fopen("passwords.txt","r");
	char **passwords;
	
	char password1[] = "client1->server#login123456c1";
	char password2[] = "client2->server#login123456c2";
	char password3[] = "client3->server#login123456c3";

	fclose(password);

	//for ( int i = 0; i<3; i++) {
		//char* fgets(char * passwords,9, FILE* password );
		
	//}
	//printf("%s \n", password1);


    // The serv_addr is the address and port number that the server will 
    // keep receiving from.
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(32000);

    bind(sockfd, 
         (struct sockaddr *) &serv_addr, 
         sizeof(serv_addr));

    while (1) {
    	//
        len = sizeof(cli_addr);
        recv_len = recvfrom(sockfd, // socket file descriptor
                 recv_buffer,       // receive buffer
                 sizeof(recv_buffer),  // number of bytes to be received
                 0,
                 (struct sockaddr *) &cli_addr,  // client address
                 &len);             // length of client address structure

        if (recv_len <= 0) {
            printf("recvfrom() error: %s.\n", strerror(errno));
            return -1;
        }


      	//Now we know there is an event from the network
      	//TODO: Figure out which event and process it according to the current state
      
printf("%s.", recv_buffer);

        char currentId[7];
        //get current ClientId
        for (int i = 0; i< 7; i++ ){
        	currentId[i] = recv_buffer[i];
        }
        //initialize the current session
        current_session = malloc(sizeof(current_session));
        for(int i = 0; i<7; i++){
        	current_session->client_id[i] = currentId[i];
        }
        current_session->client_addr = cli_addr;
        current_session->state = 0;//0: offline, 1: Login received, 2: online, 3: message forward

        current_session->token[0] = 'T';
        current_session->token[1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[random()%52];
        current_session->token[2] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[random()%52];
        current_session->token[3] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[random()%52];
        current_session->token[4] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[random()%52];
        current_session->token[5] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[random()%52];

        int event = getEvent(recv_buffer);
		if(event == -1) {
        	printf("Unrecognized input format!");
        }



        //record the last time that this sessoin is active
        current_session->last_time = time(NULL);

        if (event == 0/*login event*/) {
        	if (current_session->state == 0 /*OFFLINE state*/) {
        		//TODO: take corresponding action.
        		current_session->state == 1;//received state
        		//authentication
        		if(strcmp(recv_buffer,password1) || strcmp(recv_buffer,password2) || strcmp(recv_buffer, password3)) {
        			//anthentication pass
        			//send back success
//--->>>			
        			char send_buffer[] = "server->client1#Success";
        			send_buffer[14] = recv_buffer[6];
        			strcat(send_buffer,current_session->token);
        			printf("%s \n" , send_buffer);
        			sendto(sockfd, // socket file descriptor
                 		send_buffer,       // receive buffer
                 		sizeof(send_buffer),  // number of bytes to be received
                		 0,
                 		(struct sockaddr *) &cli_addr,  // client address
                 		&len);   
        			current_session->state = 2;//jump to online state
        			printf("Connection success with: %s\n", current_session->client_id);
        		}else{
        			//anthentication reject
        			//send back eror
//--->>>			
        			char send_buffer[] = "server->client1#Error: password does not match!";
        			send_buffer[14] = recv_buffer[6];
        			sendto(sockfd, send_buffer, sizeof(send_buffer), 0, (struct sockaddr *)&cli_addr, len);

					current_session->state = 0;//jump back to offline state
					printf("Anthentication failed.");       			
        		}

    		}
    		else if (current_session->state == 1){
    			//hand errors.
    		}
    	}
    	else if (event == 1) {

    	}

    	time_t current_time = time(NULL);
    	//Now check time of all clients.
    	//if currerent has passed 5 mins, session expires.






       /*
        if (strncmp(recv_buffer, "you->server#", strlen("you->server#")) == 0) 
            recog = true;

        //record the client input
        FILE *outputFile;
        outputFile = fopen("serverMemo.txt", "a");

        char* clientId = inet_ntoa(cli_addr.sin_addr);

        //if not recognizable, copy it to memo
        if(!recog){
            fprintf(outputFile, recv_buffer);
        }

        //trim the recv_buffer(remove the "you->server")
        strncpy(recv_buffer, &(recv_buffer[12]),1024-12);

        //if recognize, write the input into memo else change the first char to E 
        if(recog){
            fprintf(outputFile, "(%s: 32000)->server#%s", clientId, recv_buffer );          
        }
        else{
            recv_buffer[0] = 'E';
        }
        fclose(outputFile);

        //send back to
        sendto(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&cli_addr, len);
        */  

    }

    return 0;
}
