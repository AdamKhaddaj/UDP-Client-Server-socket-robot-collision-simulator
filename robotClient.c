#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"




// This is the main function that simulates the "life" of the robot
// The code will exit whenever the robot fails to communicate with the server
int main() {
	int count = 0;
  	int roboid, direction,x,y,command;
  	int turndir = -1;
  // Set up the random seed
  	srand(time(NULL));
  
  	int clientSocket, addrSize, bytesReceived;
	struct sockaddr_in serverAddr;
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
	
	addrSize = sizeof(serverAddr);
    
    printf("CLIENT: Sending \"%s\" to server.\n", "1");
    sendto(clientSocket, "1", strlen("1"), 0, (struct sockaddr *) &serverAddr, addrSize);
	
	bytesReceived = recvfrom(clientSocket, buffer, 30, 0, (struct sockaddr *) &serverAddr, &addrSize);
    buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
	
	if(strcmp(buffer,"6") == 0){ // if too many bots, close socket n exit program
	
		printf("SERVER RESPONSE: Too many robots, request denied.\n");
		close(clientSocket);
		exit(-1);
		
	}
	else{
		
		printf("Robot registered. Here's the info: %s\n",buffer);
		
		/*
		Info is extracted by making a temp string that will extract values of buffer (we know where each value is because we placed each of them in at specific indices), sprintf'ing to that string, then converting the string
		*/
		
		//COMMAND
		char Scommand[2];
		
		sprintf(Scommand,"%c",buffer[0]);
		
		command = atoi(Scommand);
		
		
		//ID
		char Sroboid[3];
		
		sprintf(Sroboid, "%c%c",buffer[1],buffer[2]);
		
		roboid = atoi(Sroboid);
		
		
		//X COORDINATE
		char Sx[4];
		
		sprintf(Sx,"%c%c%c",buffer[3],buffer[4],buffer[5]);
		
		x = atoi(Sx);
		
		//Y COORDINATE
		char Sy[4];
		
		sprintf(Sy,"%c%c%c",buffer[6],buffer[7],buffer[8]);
		
		y = atoi(Sy);
		
		
		
		//DIRECTION
		char Sdirection[5];
		
		sprintf(Sdirection,"%c%c%c%c",buffer[9],buffer[10],buffer[11],buffer[12]);
		
		direction = atoi(Sdirection);
	
		
   	 	
	}
	

  // Go into an infinite loop exhibiting the robot behavior
 	while (1) {
 		
 		
 		
 		addrSize = sizeof(serverAddr);
 		sprintf(buffer,"%d%.2d%.3d%.3d%.3d", CHECK_COLLISION, roboid, x, y, direction);
		
		sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
		bytesReceived = recvfrom(clientSocket, buffer, 30, 0, (struct sockaddr *) &serverAddr, &addrSize);
   	 	buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
   	 	
   	 	if (bytesReceived > 0) {
	   	 
	   	 	if(buffer[0] == '9'){ //if we in the middle of shutting down
				
				printf("Stuff's trying to shut down, sorry! Client being closed.\n");
	   	 		break;
	   	 	
	   	 	}
	 		
	 		if(strcmp(buffer,"5") == 0){ //if its good to move

				turndir = -1; //flag that checks if we need to pick a direction when the robot collides. when its at -1 it means we need to pick a new random direction at beginning of collision, turn this on when it moves (aka when it isnt colliding with anything)
				
				x = x+ROBOT_SPEED*cos(direction*PI/180); //given equations
				y = y+ROBOT_SPEED*sin(direction*PI/180);

				
				sprintf(buffer,"%d%.2d%.3d%.3d%.3d", STATUS_UPDATE, roboid, x, y, direction);
				


				sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
				
			}
			
			if(buffer[0] == '7' || buffer[0] == '8'){ //if colliding
				
				int diff;
				
				if(turndir == -1){ //if its the first frame of collision, pick arbitrary direction
					turndir = rand()%2;
				}
				
				if(turndir == 1){	//if it selects CCW
							
					direction += ROBOT_TURN_ANGLE;
					
				}
				else if(turndir == 0){ //if it selects CW
					
					direction -= ROBOT_TURN_ANGLE;
				
				}
				
				if(direction < -180){ //this if and else if accounts for when its passing over from 180 to -179, since the bot goes by increments of 15
						
						diff = direction + 180;
						direction = 180 + diff;
				}
					
				else if(direction > 180){
						
						diff = direction - 180;
						direction = (-180) + diff;
					
				}
				
				sprintf(buffer,"%d%.2d%.3d%.3d%.3d", STATUS_UPDATE, roboid, x, y, direction);
				
				sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
				

			}
 		
		usleep(20000);
		
  		}
	}
	close(clientSocket); 
}
