# Socket-Based-Load-Balancing

## Introduction
This project implements a client-server architecture where clients can request files from the server. The server searches for the requested files in its directory tree and returns them to the client. Multiple clients from different machines can connect to the server simultaneously.

## Section 1: Server (serverw24)
- The serverw24 and two identical copies called mirror1 and mirror2 must run on separate machines before any clients connect.
- Upon receiving a connection request, serverw24 forks a child process to service the client request.
- The crequest() function within the child process enters an infinite loop, waiting for client commands, and performs the requested actions.
- The serverw24 handles each client request in a separate child process.
  
## Section 2: Client (clientw24)
The client process runs an infinite loop, waiting for user commands. It verifies the syntax of the commands before sending them to the server.
  
### List of Client Commands:
- `dirlist -a`: Returns subdirectories in alphabetical order.
- `dirlist -t`: Returns subdirectories in the order of creation.
- `w24fn filename`: Returns information about a specific file.
- `w24fz size1 size2`: Returns a compressed file containing files within a specified size range.
- `w24ft <extension list>`: Returns a compressed file containing files with specified extensions.
- `w24fdb date`: Returns files created before a specified date.
- `w24fda date`: Returns files created after a specified date.
- `quitc`: Terminates the client process.
  
## Section 3: Alternating Between Servers (serverw24, mirror1, mirror2)
- The servers must run on separate machines.
- Initial client connections are handled by serverw24, followed by mirror1 and mirror2 in an alternating manner.
- The project ensures load balancing by distributing client connections across servers.
