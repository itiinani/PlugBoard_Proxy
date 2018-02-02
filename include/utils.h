#include <openssl/aes.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

struct ctr_state {
    unsigned char ivec[16];  /* ivec[0..7] is the IV, ivec[8..15] is the big-endian counter */
    unsigned int num;
    unsigned char ecount[16];
};
typedef struct socket_data {
  int server_socket;
  int client_socket;
  char *key;
} socket_data;

extern int RAND_bytes(unsigned char *buf, int num);
extern int init_client(char * host, char * port,char * key);
extern int init_server(char * port, char * key, char * host, char * actual_server_port);
extern int init_ctr(struct ctr_state *state, const unsigned char iv[8]);
void *server_readt(void * socketdata);
void *server_writet(void * socketdata);
void *read_decrypt(void * socks);
void *write_encrypt(void * socketdata);



