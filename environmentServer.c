#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "simulator.h"


Environment    environment;  // The environment that contains all the robots

void generateRobot(int*, int*, int*);



// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should repeatedly grab an incoming messages and process them. 
// The requests that may be handled are STOP, REGISTER, CHECK_COLLISION and STATUS_UPDATE.   
// Upon receiving a STOP request, the server should get ready to shut down but must
// first wait until all robot clients have been informed of the shutdown.   Then it 
// should exit gracefully.  
void *handleIncomingRequests(void *e) {
	char   online = 1;
	int robocounter = 0;
	//server variables
	
  	int serverSocket, clientSocket;
  	struct sockaddr_in serverAddress, clientAddr;
  	int status, addrSize, bytesRcv;
  	fd_set readfds, writefds;
  	char buffer[30];
  	
  	// Initialize the server
  	//open socket
  	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  	if (serverSocket < 0) {
  		printf("SERVER ERROR, COULD NOT OPEN SOCKET\n");
  		exit(-1);
  	}
  	
  	//setup server address
  	memset(&serverAddress, 0, sizeof(serverAddress));
  	serverAddress.sin_family = AF_INET;
  	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  	serverAddress.sin_port = htons((unsigned short) SERVER_PORT);
  	
  	//bind
  	status = bind(serverSocket, (struct sockaddr *)&serverAddress,sizeof(serverAddress));
	if(status < 0) {
		printf("SERVER ERROR, COULD NOT BIND SOCKET\n");
	}
	
  	// Wait for clients now
	while (online) {
		
		
		FD_ZERO(&readfds);
		FD_SET(serverSocket, &readfds);
		FD_ZERO(&writefds);
		FD_SET(serverSocket, &writefds);
		
		status = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
		
		if (status == 0) { // Timeout occurred, no client ready
		}
		
		else if (status < 0) {
		printf("*** SERVER ERROR: Could not select socket.\n");
		exit(-1);
		}
		
		else {
			
			addrSize = sizeof(clientAddr);
			
			bytesRcv = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddr, &addrSize);
			
			if (bytesRcv > 0) {
				
				buffer[bytesRcv] = 0;
			
				if(buffer[0] == '2'){ //if its the stop command
					
					printf("Command to stop has been recieved\n");
					
					if(environment.numRobots == 0){ //if there are no bots, close immediately
						break;
					}
					
					environment.shutDown = 1;
					
				}

				
				if(strcmp(buffer,"1") == 0){ //if its register command
					
				
					if(environment.numRobots == MAX_ROBOTS){ //if at max num of robots
					
						sprintf(buffer,"%d", NOT_OK);

						sendto(serverSocket, buffer,strlen(buffer), 0,(struct sockaddr *) &clientAddr, addrSize); //dont let it add bots and make it close the client
						

					}
					else{ //if there is room for another robot
						
						int x;
						int y;
						int direction;
						
						generateRobot(&x,&y,&direction); //use helper function
						
						int roboid = environment.numRobots+1;

						environment.robots[roboid-1].x = x;
						environment.robots[roboid-1].y = y;
						environment.robots[roboid-1].direction = direction;
						//printf("direction%d\n",direction);
						
						environment.numRobots++;
						
						sprintf(buffer,"%d%.2d%.3d%.3d%.3d", OK, roboid, x, y, direction); //put all of this info into the buffer string at exact indexes of the string, so that they can be extracted accurately later


						sendto(serverSocket, buffer,strlen(buffer), 0,(struct sockaddr *) &clientAddr, addrSize);

					}
				}
				if(buffer[0] == '3'){ //check collision
				
					if (environment.shutDown) { //check for shutdown in collision since, if collision is being ran, the bot will exist and it will pass thru 1 at a time
					
						sendto(serverSocket, "9", strlen("9"), 0, (struct sockaddr *) &clientAddr, addrSize);
						environment.numRobots--;

						if (environment.numRobots == 0) {
							online = 0;
						}
					}
					
					//use extraction method as explained in robot client
					
					//COMMAND
					char Scommand[2];
					
					sprintf(Scommand,"%c",buffer[0]);
					
					int command = atoi(Scommand);
					
					
					//ID
					char Sroboid[3];
					
					sprintf(Sroboid, "%c%c",buffer[1],buffer[2]);
					
					int roboid = atoi(Sroboid);
					
					
					//X COORDINATE
					char Sx[4];
					
					sprintf(Sx,"%c%c%c",buffer[3],buffer[4],buffer[5]);
					
					int x = atoi(Sx);
					
					//Y COORDINATE
					char Sy[4];
					
					sprintf(Sy,"%c%c%c",buffer[6],buffer[7],buffer[8]);
					
					int y = atoi(Sy);
					

					//DIRECTION
					char Sdirection[5];
					
					sprintf(Sdirection,"%c%c%c%c",buffer[9],buffer[10],buffer[11],buffer[12]);
					
					int direction = atoi(Sdirection);
										
					float newX = x+ROBOT_SPEED*cos((direction*PI)/180); //given equations
				    float newY = y+ROBOT_SPEED*sin((direction*PI)/180); 

					
					if(environment.numRobots > 1){
						int skipper = 0; //flag for if we wanna stop and do rotation stuff
						
						for(int i = 0 ; i < environment.numRobots ; i++){
							
							if(i != roboid-1){ //dont let it check for itself

								float dist = sqrt(pow((newX - environment.robots[i].x),2) + pow((newY - environment.robots[i].y),2));  //use same circle overlap check equation as in the generate_robots helper function
								
								float raddist = ROBOT_RADIUS*2;


								if(dist <= raddist){ //if are colliding
									
									sendto(serverSocket, "8",strlen("8"), 0,(struct sockaddr *) &clientAddr, addrSize);
									skipper = 1;
									break; //go straight to doing collision stuff
								}
							}
						}
						
						if(skipper){
							continue;
						}
					}
					
					
					if((newX<ROBOT_RADIUS ||newX>ENV_SIZE-ROBOT_RADIUS)||(newY<ROBOT_RADIUS ||newY>ENV_SIZE-ROBOT_RADIUS)){ //checking bounds of environment while accounting for robot radius


				            sendto(serverSocket, "7",strlen("7"), 0,(struct sockaddr *) &clientAddr, addrSize); //cant go
				            continue;
				    }
				    
				    
				    else{

				    	sendto(serverSocket, "5",strlen("1"), 0,(struct sockaddr *) &clientAddr, addrSize); //good to go
				    }
				
				}
				
				if(buffer[0] == '4'){ //STATUS UPDATE FOR WHEN THE ROBOT MOVES
					
					//same extraction method as explained in robot client
					
					//COMMAND
					char Scommand[2];
					
					sprintf(Scommand,"%c",buffer[0]);
					
					int command = atoi(Scommand);
					
					
					//ID
					char Sroboid[3];
					
					sprintf(Sroboid, "%c%c",buffer[1],buffer[2]);
					
					int roboid = atoi(Sroboid);
					
					
					//X COORDINATE
					char Sx[4];
					
					sprintf(Sx,"%c%c%c",buffer[3],buffer[4],buffer[5]);
					
					int x = atoi(Sx);
					
					//Y COORDINATE
					char Sy[4];
					
					sprintf(Sy,"%c%c%c",buffer[6],buffer[7],buffer[8]);
					
					int y = atoi(Sy);
					
					
					
					
					//DIRECTION
					char Sdirection[5];
					
					sprintf(Sdirection,"%c%c%c%c",buffer[9],buffer[10],buffer[11],buffer[12]);
					
					int direction = atoi(Sdirection);
					
					//update values
					environment.robots[roboid-1].x = x;
					environment.robots[roboid-1].y = y;
					environment.robots[roboid-1].direction = direction;
					

					
				}
			}
		}	
  	}
  	close(serverSocket);
  	printf("Server is now closed! \n");
  	// ... WRITE ANY CLEANUP CODE HERE ... //
}


void generateRobot(int *x, int *y, int *direction){
	
	int startover = 0; //flag for if it gets spawned on another robot and needs to restart
	
	int tempx = rand() % (ENV_SIZE - (2*ROBOT_RADIUS)) + ROBOT_RADIUS; //random number accounting for radius
	int tempy = rand() % (ENV_SIZE - (2*ROBOT_RADIUS)) + ROBOT_RADIUS;
	
	for(int i = 0 ; i < environment.numRobots ; i++){
	
		int dist = pow((tempx - environment.robots[i].x),2) + pow((tempy - environment.robots[i].y),2);  //equation for checking overlapping circles
		
		int raddist = pow((ROBOT_RADIUS*2),2); 
		
		if (dist == raddist){} //they're touching but not intersecting (should be fine) 

		else if (dist > raddist){} //they're not touching
		
		else{ //they're intersecting, get new x and y
		    startover = 1;
		    generateRobot(x,y,direction); //call function again, current function in stack will return empty and do nothing
		}
	}
	if(startover){
		return;
	}
	else{
		int tempdirection = (rand() % 361) - 180;

		*x = tempx; //set values
		*y = tempy;
		*direction = tempdirection;
		return;
	}

}








int main() {

	pthread_t redrawer, requesttaker;
	srand(time(NULL));
	
	environment.shutDown = 0;
	
	pthread_create(&redrawer, NULL, redraw, &environment);
	pthread_create(&requesttaker, NULL, handleIncomingRequests, &environment);
	
	pthread_join(requesttaker, NULL);
	//no point in joining other thread since redraw shouldnt need to wait on requesttaker
}













