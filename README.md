<h1 align="center">C-Multi-Threaded-Server-Client</h1>
<h3 align="center">Oprating Systems Course</h3>

## Description:
#### An implementation of a TCP multi-threaded server-client API, such that the server can simultaneously respond to multiple clients.
On the server side, the user selects the following at the beginning of the program run:
* The port number the server will listen on.
* The number of worker threads that should be created within the web server (a positive integer).
* The number of request connections can be accepted at one time. (a positive integer).
Might be more or less than the threads number.
* The scheduling algorithm to be performed. Must be one of "block", "dt", "dh", or "random".  

## Download:
    git clone https://github.com/oradbarel/C-Multi-Threaded-Server-Client.git
    
## Compilation:
Once downloaded, do the following:

    cd C-Multi-Threaded-Server-Client
    make

## Running:
### On the server side:

    ./server [portnum] [threads] [queue_size] [schedalg]
for example:

    ./server 8002 10 20 dt
    
### On the client side:

    ./client [hostname] [port] [filename]
for example:

    ./client localhost 8002 home.html
