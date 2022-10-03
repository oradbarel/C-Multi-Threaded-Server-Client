<h1 align="center">C-Multi-Threaded-Server-Client</h1>
<h3 align="center">Oprating Systems Course</h3>

## Description:
#### An implementation of a TCP multi-threaded server-client API, such that the server can simultaneously respond to multiple clients.
On the server side, the user selects the following at the beginning of the program run:
* The port number the server will listen on.
* The number of worker threads that should be created within the web server (a positive integer).
* The number of request connections can be accepted at one time (a positive integer).
Might be more or less than the threads number.
* The scheduling algorithm to be performed. Must be one of "block", "dt", "dh", or "random".

On the client side, the user sould run the program with the IP and PORT numbers of the server.

## Client side demonstration:

### An example of a successful running:
<img src="https://user-images.githubusercontent.com/75837421/188899306-606afe26-1aea-44d0-860d-b411627f0276.jpg" >

### An example of a "404 error":
<img src="https://user-images.githubusercontent.com/75837421/188899331-89813204-8265-4ee1-be87-7d67b83facfa.jpg" >

## Download:
    git clone https://github.com/oradbarel/C-Multi-Threaded-Server-Client.git
    
## Build:
Once downloaded, do the following:

    cd C-Multi-Threaded-Server-Client/src
    make

## Run:
### On the server side:

    ./server [portnum] [threads] [queue_size] [schedalg]
for example:

    ./server 8002 10 20 dt
    
### On the client side:

    ./client [hostname] [port] [filename]
for example:

    ./client localhost 8002 home.html
    

