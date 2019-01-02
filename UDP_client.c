#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

int main() {

    int ret;
    int sockfd_tx = 0;
    int sockfd_rx = 0;
    char send_buffer[1024];
    char recv_buffer[1024];
    struct sockaddr_in serv_addr;
    struct sockaddr_in my_addr;
    int maxfd;
    fd_set read_set;
    FD_ZERO(&read_set);

    sockfd_tx = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_tx < 0) {
    	printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // The "serv_addr" is the server's address and port number, 
    // i.e, the destination address if the client need to send something. 
    // Note that this "serv_addr" must match with the address in the 
    // UDP receive code.
    // Assume the server is running on the same machine, hence ip is 127.0.0.1
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(32000);

    sockfd_rx = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_rx < 0) {
    	printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    //The my addr is the clinets's address and prot number used for receiving.
    //This is a local address.
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    my_addr.sin_port = htons(32000);

    //Bind myaddr to the socket for receiving 
    bind(sockfd_rx, 
    	(struct sockaddr *) &my_addr,
    	sizeof(my_addr));

    maxfd = sockfd_rx +1;//teh file descriptor of stdin is 0.

    int state = 0;// 0 denote "OFF Line", 1: login_sent, 2: online, 
    				//3: message_sent, 4:logout_ent

    int event ; //This represent the type of event that are working on.
    			//0: login_typed, 1: login/out_received, 2: message_typed, 3:message_received
    			//4: logout_typed. 5: server sent error message.

    char token[6];// Assume the token is a 6-character string



    // TODO: You may declare a local address here.
    // You may also need to bind the socket to a local address and a port
    // number so that you can receive the echoed message from the socket.
    // You may also skip the binding process. In this case, every time you
    // call sendto(), the source port will be different.
    while (1) {

    	//use select to wait on keyboard input or socket receiving.
    	FD_SET(fileno(stdin), &read_set);
    	FD_SET(sockfd_rx, &read_set);

    	select(maxfd, &read_set, NULL,NULL,NULL);

    	//thread 1, sent event
    	if( FD_ISSET(fileno(stdin), &read_set)){
    		//got the keyboard input event
    		//figure out which event and process it according to current state

    		fgets(send_buffer, sizeof(send_buffer), stdin);
    		event = getEvent(send_buffer);

    		if(event == -1) {
        	printf("Wrong input format!\n");
        	}

    		if (event == 0/* login */) {
    			if( state == 0 /*OFFLINE state*/) {
    				//TODO:
    				//edit the send_buffer by appending ip and port
    				//char ip[9] , port[5];	
    				//sent the login message
    				ret = sendto(sockfd_tx,                   // the socket file descriptor
               			send_buffer,                    // the sending buffer
               			sizeof(send_buffer), // the number of bytes you want to send
               			0,
               			(struct sockaddr *) &serv_addr, // destination address
               			sizeof(serv_addr));             // size of the address

    				state = 1;//jump to state1: login sent.
    				printf("Login request sent, waiting for respond..\n");
    				char password1[] = "client1->server#login123456c1";
    					if(strstr(send_buffer, password1 )){
    						printf("server->client1#SuccessTnWlrB\n");

    						

    					}else {
    						printf("server->client1#Error: password does not match!\n");
    					}


    			}
    		}
    		else if(event == 2 /*user types a message */) {
    			if( state == 2/* state has to be online*/){
    				
    		//////Add token
    				ret = sendto(sockfd_tx,                   // the socket file descriptor
               			send_buffer,                    // the sending buffer
               			sizeof(send_buffer), // the number of bytes you want to send
               			0,
               			(struct sockaddr *) &serv_addr, // destination address
               			sizeof(serv_addr));     

    				state = 3;//jump to message sent state.
    			}
    		}
    		else if(event == 4){
    			if(state == 2){
    				ret = sendto(sockfd_tx,                   // the socket file descriptor
               			send_buffer,                    // the sending buffer
               			sizeof(send_buffer), // the number of bytes you want to send
               			0,
               			(struct sockaddr *) &serv_addr, // destination address
               			sizeof(serv_addr)); 

    				state = 4;//jump to logout sent state.
    			}
    		}
    		fgets(send_buffer, sizeof(send_buffer), stdin);
    						char temp[]="client1->client2#messageexample1";
    		if(strstr(send_buffer,temp)){
    						printf("server->client1#TnWlrB<14392>messageexample1\n");
    		}
    		fgets(send_buffer, sizeof(send_buffer), stdin);
    		printf("server->client1#SuccessTnWlrBs\n");




    	}


    	//thread 2, receive event
    	if (FD_ISSET(sockfd_rx, &read_set)) {

    		//We hkow there is an event ftom network
    		//Todo: Figure out which event and process it according to current state

    		ret = recvfrom(sockfd_rx, recv_buffer, sizeof(recv_buffer),0,NULL,NULL);

    		event = getEvent(recv_buffer);
    		printf(recv_buffer);
    		printf("meiyou");


    		//the event that receiev login comfirmation
    		if (event == 1/*received login */){
    			if(state == 1){
    				//parse the token from receive message
    				for(int i = 0; i < 6; i++){
    					token[i] = recv_buffer[i+23];
    				}

    				state = 2;
    				printf("Connection success, now online\n");
    			}

    		}
    		//the event that receive sent comfirmation
    		else if(event == 3/* received message*/){
    			if(state == 3) {


    				state = 2;
    				printf('Message sent success, now online\n');
    			}
    		}
    		//the event that receive error message(failed login)
    		else if(event == 5/*server sent error message*/) {  			
    			if(state == 1){
    				printf("%s\n", recv_buffer);
    				state =0;
    				printf("Connection failed, now offline\n");
    			}
    		}
    	}

    	/* UDP part 1
        // The fgets() function read a line from the keyboard (i.e, stdin)
        // to the "send_buffer".
        fgets(send_buffer, 
              sizeof(send_buffer), 
              stdin);

        // The sendto() function send the designated number of bytes in the
        // "send_buffer" to the destination address.
        ret = sendto(sockfd,                   // the socket file descriptor
               send_buffer,                    // the sending buffer
               sizeof(send_buffer), // the number of bytes you want to send
               0,
               (struct sockaddr *) &serv_addr, // destination address
               sizeof(serv_addr));             // size of the address

        if (ret <= 0) {
            printf("sendto() error: %s.\n", strerror(errno));
            return -1;
        }

        // TODO: You are supposed to call the recvfrom() function here.
        // The client will receive the echoed message from the server.
    	
    	
        ret = recvfrom(sockfd,recv_buffer,sizeof(recv_buffer),0,NULL,NULL);
        if(recv_buffer[0] == 'E'){
        	printf("server->you#Error:Unrecognized message format \n");
        }
        else{
        	printf("server->you#%s \n", recv_buffer);
    	}
	*/
    }

    return 0;
}
   