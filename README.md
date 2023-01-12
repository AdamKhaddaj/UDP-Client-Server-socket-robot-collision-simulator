# UDP-Client-Server-socket-robot-collision-simulator
This project implements a UDP Client / Server socket that uses multiple threads to allow
multiple clients (which appear small robots) to connect to a simulator (a 2D plane), all of which run as
their own processes.

Once registered, the robots send and receive information from the server in real time to
determine whether they are going to collide with another robot or wall, and then correct their
direction to allow them to continue moving inside the plane

Running instructions:

Open terminal in this directory and run the 'make' command to compile.

To start the server, type './environmentServer &'

The two commands you can use are './robotClient &' to create a robot, and './stop/ to shut down the server and all associated processes.

