#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


int main() {
	int clientSocket, addrSize, bytesReceived;
	struct sockaddr_in serverAddr;
	char inStr[80]; // stores user input from keyboard
	char buffer[80]; // stores sent and received data
  
  	clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSocket < 0) {
	printf("*** CLIENT ERROR: Could open socket.\n");
	exit(-1);
	}
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddr.sin_port = htons((unsigned short) SERVER_PORT);
	
  // this code is pretty straightforward, if stop ever gets called, send 2 to the server letting it now stuffs gonna start shutting down

	
	addrSize = sizeof(serverAddr);

	printf("CLIENT: Sending stop to server.\n");
	sendto(clientSocket, "2", strlen("2"), 0,(struct sockaddr *) &serverAddr, addrSize);
	

	close(clientSocket); 


}
