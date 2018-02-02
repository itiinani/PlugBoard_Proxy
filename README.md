# PBProxy

#Test Environment-------------------------------------------------------------------------

Linux 3.13.0-135-generic x86_64

Distributor ID:	Ubuntu
Description:	Ubuntu 14.04.5 LTS
Release:	14.04
Codename:	trusty

#Compiler----------------------------------------------------------------------------------

gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4

Command line and options to run the program -

1. To compile: $make

2. To start server: 
$sudo ./pbproxy -k mykey -l pbproxy_port ssh_host_address ssh_port

For e.g : $sudo ./pbproxy -k mykey -l 5004 localhost 22

3. To start client:
$ssh -o "ProxyCommand=./pbproxy -k mykey pbproxy_server_address pbproxy_port" localhost

For e.g : $ssh -o "ProxyCommand=./pbproxy -k mykey localhost 5004" localhost

NOTE: the -k option must be provided with the filename containing the key.
If no option for key is provided then program takes default key as "ABCD0016ABCD0016".

#Project Overview----------------------------------------------------------------------------------
'pbproxy', adds an extra layer of encryption to connections towards TCP services. 
Instead of connectingdirectly to the service, clients connect to pbproxy (running on 
the sameserver), which then relays all traffic to the actual service. Before relaying
the traffic, pbproxy *always* decrypts it using a static symmetric key. This means 
that if the data of any connection towards the protected server is not properly 
encrypted, then it will turn into garbage before reaching the protected service.

Clients who want to access the protected server should proxy their traffic
through a local instance of pbroxy, which will encrypt the traffic using the
same symmetric key used by the server. In essence, pbproxy can act both as
a client-side proxy and as server-side reverse proxy.

#Project Structure --------------------------------------------------------------------------------

#Files
pbproxy.c - this contains the main function.
            -parses the arguments and options on command line.
            -starts server if -l is passed.
            -starts client if -l is not passed.
client-server.c: this is a common file contains code related to client and server both.
utils.h - this is the header files,contains socket_data structure,ctr-state 
          structure and declarations of functions in client-server.c

#Functions------------------------------------------------------------------------------------
#Client-side pbproxy service-
extern int init_client(char * host, char * port,char * key);
- this function starts client, creates socket to listen to client-side proxy 
  connection and creates read and write threads.

void *read_decrypt(void * socks);
- this function is called when read thread is created by client-side pbproxy service.
  It reads the response from server,decrypts it and sends to client.

void *write_encrypt(void * socketdata);
-this function is called when write thread is created by client-side pbproxy service.
 It received the data from client(from STDIN),encrypts it and sends it to ssh_server.

#Server-side pbproxy service-
extern int init_server(char * port, char * key, char * host, char * actual_server_port);
 - this function starts server,creates socket to listen to client 
   and if there is any request from client,connects to ssh server on port 22,
   create read and write threads to read and write data from ssh server.

void *server_readt(void * socketdata);-
 - this function is called when read thread is created from pbproxy server.
   It checks for data from client,decrypts it and passes to the ssh server.

void *server_writet(void * socketdata);
-  this function is called when write thread is created from pbproxy server.
   It received the response from ssh_server,encrypts it and sends it to client. 

#common functions
extern int RAND_bytes(unsigned char *buf, int num); -
- to generate random bytes for IV.

extern int init_ctr(struct ctr_state *state, const unsigned char iv[8]);
 - this function initializes counter state for AES encryption.

#References--------------------------------------------------------------------------------------
1)http://www3.cs.stonybrook.edu/~mikepo/CSE508/2017/hw/CSE508_2017_hw3.txt
2)http://www.gurutechnologies.net/blog/aes-ctr-encryption-in-c/
3)https://opensource.apple.com//source/OpenSSL/OpenSSL-22/openssl/crypto/aes/aes_ctr.c
4)http://www.geeksforgeeks.org/socket-programming-cc/

