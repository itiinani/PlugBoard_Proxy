#include <ctype.h>
#include <utils.h>

char* getkey(char* keyfile) {
	FILE *ifp = fopen(keyfile, "r");
	if (ifp == NULL) {
		fprintf(stderr, "Error in opening key file.\n");
		exit(1);
	}
	char str[4096] = { 0 };
	fscanf(ifp, "%s", str);
	char *buffer = (char*) malloc(sizeof(char) * 17);
	memset(buffer, 17, 0);
	int i=0;
	for (i = 0; i < 32; i += 2) {
		char tmp[3] = { str[i], str[i + 1], '\0' };
		int num = (int) strtol(tmp, NULL, 16);
		buffer[i / 2] = num;
	}
	fprintf(stderr,"%s", buffer);
	return buffer;
}

int main(int argc, char **argv) {
	char *keyfile = NULL;
	char *server_port = NULL;
	char *server_addr = NULL;
	char *serverd_port = NULL;
	int server_flag = 0;
	int keyFlag = 0;
	char c;
	int index;
	while ((c = getopt(argc, argv, "k:l:")) != -1)
		switch (c) {
		case 'k':
			keyfile = optarg;
			keyFlag = 1;
			break;
		case 'l':
			server_flag = 1;
			server_port = optarg;
			break;
		case '?':
			if (optopt == 'k' || optopt == 'l')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			return (0);
			break;
		}

	char* key = (char*) malloc(sizeof(char) * 4096);
	if (1 == keyFlag) {
		key = getkey(keyfile);
	}
	if (0 == keyFlag) {
		key = "ABCD0016ABCD0016"; //default key if key attribute not passed through command line
	}
	int args = 0;
	for (index = optind; index < argc; index++) {
		args++;
	}
	if (args > 0) {
		server_addr = argv[optind];
	}
	if (args == 2) {
		serverd_port = argv[optind + 1];
	}
	if (server_flag == 1) {
		init_server(server_port, server_addr, serverd_port, key);
	} else {
		init_client(server_addr, serverd_port, key);
	}
	free(key);
	return 0;
}
