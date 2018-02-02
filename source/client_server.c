#include <utils.h>


/* start server */
int init_server(char *port,char * host, char* actual_server_port,char * key) {
	struct sockaddr_in client_addr;
	int option = 1;
	int client_socket, new_socket;
	int clientaddrlen = sizeof(client_addr);
	char buffer[8192] = { 0 };
	pthread_t rthread, wthread;
    int portno = atoi(port);
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(portno);

	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			&option, sizeof(option))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	if (bind(client_socket, (struct sockaddr *) &client_addr,
			sizeof(client_addr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(client_socket, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Server started.\n");
	while (1) {
		new_socket = accept(client_socket, (struct sockaddr *) &client_addr,
				(socklen_t*) &clientaddrlen);
		if (new_socket < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		} else {
			int server_socket = 0;
			struct sockaddr_in server_addr;
			if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				printf("\n socket failed. \n");
				return -1;
			}
			memset(&server_addr, '0', sizeof(server_addr));
            int aspno = atoi(actual_server_port);
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(aspno);
			//printf("%d",aspno);

			struct hostent *server = gethostbyname(host);
			if (server == NULL) {
				printf("Host not found\n");
				return -1;
			}
			bcopy((char *) server->h_addr,
			(char *)&server_addr.sin_addr.s_addr,
			server->h_length);

			if (connect(server_socket, (struct sockaddr *) &server_addr,
					sizeof(server_addr)) < 0) {
				printf("\nUnable to connect to ssh server \n");
				return -1;
			}
			if (server_socket < 0) {
				return -1;
			}
			socket_data *sd = (socket_data *) malloc(sizeof(socket_data));
			sd->server_socket = server_socket;
			sd->client_socket = new_socket;
			sd->key = key;
			int iret1 = pthread_create(&rthread, NULL, server_readt,
					(void *) sd);
			int iret2 = pthread_create(&wthread, NULL, server_writet,
					(void *) sd);
		}
	}

	return 0;
}

/* read message from client,decrypt and send to actual server.*/
void *server_readt(void * socketdata) {
	unsigned char iv[8];
	socket_data * sd = (socket_data *) socketdata;
	int clientsock = sd->client_socket;
	int sshsock = sd->server_socket;
	char* key = sd->key;
	struct ctr_state *state = (struct ctr_state *) malloc(
			sizeof(struct ctr_state));
	read(clientsock, iv, 8);
	init_ctr(state, iv);
	AES_KEY *aes_key = (AES_KEY *) malloc(sizeof(AES_KEY));
	if (AES_set_encrypt_key(key, 128, aes_key) < 0) {
		printf("Could not set encryption key \n");
		exit(1);
	}
	while (1) {
		char buffer[8192] = { 0 };
		int readsize = read(clientsock, buffer, 8192);
		if (readsize <= 0)
			break;
		if (readsize > 0) {
			char decrypt[8192] = { 0 };
			AES_ctr128_encrypt(buffer, decrypt, readsize, aes_key, state->ivec,
					state->ecount, &(state->num));
			send(sshsock, decrypt, readsize, 0);
		}
	}
}

/* encrypt response from server and send it to client */
void *server_writet(void * socketdata) {
	unsigned char iv[8];
	struct ctr_state *state = (struct ctr_state *) malloc(
				sizeof(struct ctr_state));
	socket_data * sd = (socket_data *) socketdata;

	int client_socket = sd->client_socket;
	int server_socket = sd->server_socket;
	char * key = sd->key;

	if (!RAND_bytes(iv, 8)) {
		printf("Could not generate IV!\n");
		exit(EXIT_FAILURE);
	}
	init_ctr(state, iv);
	send(client_socket, iv, strlen(iv), 0);
	AES_KEY *aes_key = (AES_KEY *) malloc(sizeof(AES_KEY));
	if (AES_set_encrypt_key(key, 128, aes_key) < 0) {
		printf("could not set encryption key \n");
		exit(1);
	}
	while (1) {
		char buffer[8192] = { 0 };
		int readsize = read(server_socket, buffer, 8192);
		if (readsize > 0) {
			char encry[8192] = { 0 };
			AES_ctr128_encrypt(buffer, encry, readsize, aes_key, state->ivec,
					state->ecount, &(state->num));
			send(client_socket, encry, readsize, 0);
		}
	}
}



/* start client */
int init_client(char *host,char *port,char *key) {
	struct sockaddr_in proxy_addr;
	int client_socket = 0;
	char buffer[8192] = { 0 };
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}
	int portno = atoi(port);
	memset(&proxy_addr, '0', sizeof(proxy_addr));
	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_port = htons(portno);

	struct hostent *server = gethostbyname(host);
	if (server == NULL) {
		printf("Host Unknown!\n");
		return -1;
	}
	bcopy((char *) server->h_addr, (char *)&proxy_addr.sin_addr.s_addr, server->h_length);
	if (connect(client_socket, (struct sockaddr *) &proxy_addr, sizeof(proxy_addr))
			< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	pthread_t rthread, wthread;
	socket_data *sd = (socket_data *) malloc(sizeof(socket_data));
	sd->server_socket = 0;
	sd->client_socket = client_socket;
	sd->key = key;

	int t1 = pthread_create(&rthread, NULL, read_decrypt, (void *) sd);
	int t2 = pthread_create(&wthread, NULL, write_encrypt, (void *) sd);

	pthread_join(rthread, NULL);
	pthread_join(wthread, NULL);
	printf("read thread returns: %d %d\n", t1, t2);

	return 0;
}
/*initialize counter state for encryption/decryption*/
int init_ctr(struct ctr_state *state, const unsigned char iv[8]) {
	state->num = 0;
	memset(state->ecount, 0, 16);
	memset(state->ivec + 8, 0, 8);
	memcpy(state->ivec, iv, 8);
}
/* read from serverSocket, decrypt  it and send to client*/
void *read_decrypt(void * socks) {
	AES_KEY *aes_key = (AES_KEY *) malloc(sizeof(AES_KEY));
	socket_data *sd = (socket_data *) socks;
	int client_socket = sd->client_socket;
	int server_socket = sd->server_socket;
	char * key = sd->key;
	unsigned char iv[8];
	struct ctr_state * state = (struct ctr_state *) malloc(
			sizeof(struct ctr_state));
	read(client_socket, iv, 8);
	init_ctr(state, iv);
	if (AES_set_encrypt_key(key, 128, aes_key) < 0) {
		printf("Error while AES encrypt set. \n");
		exit(1);
	}
	while (1) {
		char buffer[8192] = { 0 };
		int n = read(client_socket, buffer, 8192);
		if (n > 0) {
			char decrypted_text[8192] = { 0 };
			AES_ctr128_encrypt(buffer, decrypted_text, n, aes_key, state->ivec,
					state->ecount, &(state->num));
			write(STDOUT_FILENO, decrypted_text, n);
		}
	}
}
/*read from STDIN and send to client side PBProxy instance where it is encrypted and sent to server*/
void *write_encrypt(void * socketdata) {
	socket_data * sd = (socket_data *) socketdata;
	int client_socket = sd->client_socket;
	int server_socket = sd->server_socket;
	char* key = sd->key;
	unsigned char iv[8];
	AES_KEY *aes_key = (AES_KEY *) malloc(sizeof(AES_KEY));
	struct ctr_state * state = (struct ctr_state *) malloc(
			sizeof(struct ctr_state));
	if (!RAND_bytes(iv, 8)) {
		printf("Error while generating IV!\n");
	}
	if (AES_set_encrypt_key(key, 128, aes_key) < 0) {
		printf("Error while AES encrypt set.\n");
		exit(1);
	}
	init_ctr(state, iv);
	send(client_socket, iv, strlen(iv), 0);
	while (1) {
		char buffer[8192] = { 0 };
		int n = read(STDIN_FILENO, buffer, 8192);
		if (n > 0) {
			char encrypted_text[8192] = { 0 };
			AES_ctr128_encrypt(buffer, encrypted_text, n, aes_key, state->ivec,
					state->ecount, &(state->num));
			send(client_socket, encrypted_text, n, 0);
		}
	}
}





