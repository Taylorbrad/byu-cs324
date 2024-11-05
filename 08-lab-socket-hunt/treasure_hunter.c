// Replace PUT_USERID_HERE with your actual BYU CS user id, which you can find
// by running `id -u` on a CS lab machine.
#define USER_ID 1823691656

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include "sockhelper.h"

int verbose = 0;

void print_bytes(unsigned char *bytes, int byteslen);
void fill_buf_with_message(unsigned char * buf, int level, int seed, unsigned int USERID);

int main(int argc, char *argv[]) {
	const int BUFSIZE = 8;
	const unsigned int USERID = USER_ID;

	// int port = atoi(argv[2]);
	int level = atoi(argv[3]);
	int seed = atoi(argv[4]);

	char *serverString = argv[1];
	char *portString = argv[2];

	// printf("\nServer: %s\nPort: %s\nLevel: %d\nID: %d\nSeed: %d\n", serverString, portString, level, USERID, seed);

	unsigned char buf[BUFSIZE];
	bzero(buf, BUFSIZE);

	fill_buf_with_message(buf, level, seed, USERID);

	// print_bytes(buf, 8);
	// print_bytes();
	// htons
	// ntohs

	// htonl
	// ntohl

	int sockfd = 5;
	int addr_fam = AF_UNSPEC;
	struct addrinfo hints, *servinfo;
	int rv;
	// int numbytes;
	socklen_t addr_len;

	struct sockaddr_storage remote_addr_ss;
	struct sockaddr* remote_addr = (struct sockaddr*)&remote_addr_ss;
	char remote_ip[INET6_ADDRSTRLEN];
	unsigned short remote_port;

	struct sockaddr_storage local_addr_ss;
	struct sockaddr* local_addr = (struct sockaddr*)&local_addr_ss;
	char local_ip[INET6_ADDRSTRLEN];
	unsigned short local_port;



	// sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(serverString, portString, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	struct addrinfo* rp;
	int socktype = 5;

	for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
		// getnameinfo(p->ai_addr, p->ai_addrlen,buf, MAXLINE, NULL, 0, flags);
		sockfd = socket(rp->ai_family, rp->ai_socktype, 0);

		if (sockfd < 0) {
			continue;
		}

		addr_fam = rp->ai_family;
		addr_len = rp->ai_addrlen;
		socktype = rp->ai_socktype;
		memcpy(remote_addr, rp->ai_addr, sizeof(struct sockaddr_storage));

		parse_sockaddr(remote_addr, remote_ip, &remote_port);
		// fprintf(stderr, "Connecting to %s:%d (addr family: %d)\n",
		// 				remote_ip, remote_port, addr_fam);

		// close(sockfd);
	}

	// if (rp == NULL) {
	// 	fprintf(stderr, "Could not connect \n");
	// 	exit(EXIT_FAILURE);
	// }

	freeaddrinfo(servinfo);

	addr_len = sizeof(struct sockaddr_storage);
	// int s = getsockname(sockfd, local_addr, &addr_len);

	parse_sockaddr(local_addr, local_ip, &local_port);
	// fprintf(stderr, "Local socket: %s %d %d\n", local_ip, local_port, addr_len);


	ssize_t nwritten = sendto(sockfd, buf, sizeof(buf), 0, remote_addr, addr_len);
	if (nwritten < 0) {
		perror("suace");
	}

	unsigned char recvBuf[256];
	ssize_t nread = recvfrom(sockfd, recvBuf, sizeof(recvBuf), 0, remote_addr, &addr_len);
	recvBuf[nread] = '\0';

	if (nread < 0) {
		perror("sauce");
	}
	// printf("receive: %s", recvBuf);
	// printf("bytes read: %ld", nread);
	// print_bytes(recvBuf, nread);

	unsigned char treasure[1024];
	unsigned int treasureLen = 0;

	while (1) {

		// printf("while");
		// fflush(stdout);
		unsigned char chunkLength;
        unsigned char treasureChunk[128];
        unsigned char opCode;
        unsigned short opParam;
        unsigned int nonce;

        memcpy(&chunkLength, recvBuf, 1);

		if (chunkLength == 0) {
			break;
		}

		if (chunkLength > 127) {
			perror("sauce2");
			exit(1);
		}

        memcpy(treasureChunk, &recvBuf[1], chunkLength);
        memcpy(&opCode, &recvBuf[chunkLength+1], 1);
        memcpy(&opParam, &recvBuf[chunkLength+2], 2);
        memcpy(&nonce, &recvBuf[chunkLength+4], 4);

        nonce = ntohl(nonce);

		if (opCode == 1) {
			opParam = ntohs(opParam);
			populate_sockaddr(remote_addr, addr_fam, remote_ip, opParam);
		}
		else if (opCode == 2) {
			opParam = ntohs(opParam);
			close(sockfd);
			sockfd = socket(addr_fam, socktype, 0);
			populate_sockaddr(local_addr, addr_fam, NULL, opParam);
			if (bind(sockfd, local_addr, sizeof(struct sockaddr_storage)) < 0) {
				perror("bind()");
			}
		}
		else if (opCode == 3) {
			// printf("opparam: %d", opParam);
			opParam = ntohs(opParam);

			unsigned int sum = 0;

			for (int i = 0; i < opParam; i++) {
				struct sockaddr_storage new_remote_addr_ss;
                struct sockaddr* new_remote_addr = (struct sockaddr*)&new_remote_addr_ss;
                char new_remote_ip[INET6_ADDRSTRLEN];
                unsigned short new_remote_port;
				unsigned char temp[256];

				recvfrom(sockfd, temp, 256, 0, new_remote_addr, &addr_len);
				// populate_sockaddr(new_remote_addr, new_remote_ip, NULL, new_remote_port);
				parse_sockaddr(new_remote_addr, new_remote_ip, &new_remote_port);
				// new_remote_port = ntohs(new_remote_port);
				// printf("%d\n", new_remote_port);
				sum += new_remote_port;
				// fflush(stdout);
			}
			nonce = sum;
		}

		memcpy(&treasure[treasureLen], treasureChunk, chunkLength); //append treasure
		treasureLen += chunkLength;

        // treasureChunk[chunkLength + 1] = '\0';

        opParam = ntohs(opParam);
        // nonce += 1;


        // printf("extracted: \nlength: %d\n chunk: %s\n opcode: %d\n param: %x\n nonce: %x\n", chunkLength, treasureChunk, opCode, opParam, nonce);

        nonce = htonl(nonce + 1);

        // print_bytes(&nonce, 4);
        // printf("\n%x", nonce);

        nwritten = sendto(sockfd, &nonce, sizeof(nonce), 0, remote_addr, addr_len);
        if (nwritten < 0) {
        	perror("suace");
        }

        // unsigned char recvBuf2[256];
        nread = recvfrom(sockfd, recvBuf, sizeof(recvBuf), 0, remote_addr, &addr_len);
        recvBuf[nread] = '\0';

        if (nread < 0) {
        	perror("sauce");
        }

        // print_bytes(recvBuf, nread);
	}
	treasure[treasureLen] = '\0';
	// print_bytes(&treasure, treasureLen);
	printf("%s\n", treasure);
}

void fill_buf_with_message(unsigned char * buf, int level, int seed, unsigned int USERID) {
	unsigned short newSeed = htons(seed);
	unsigned int newUserid = htonl(USERID);
	// printf("1:%d", level);
	// printf("\n2:%d", level);
	memcpy(&buf[1], &level, 1);

	memcpy(&buf[2], &newUserid, 4);

	memcpy(&buf[6], &newSeed, 2);

}


void print_bytes(unsigned char *bytes, int byteslen) {
	int i, j, byteslen_adjusted;

	if (byteslen % 8) {
		byteslen_adjusted = ((byteslen / 8) + 1) * 8;
	} else {
		byteslen_adjusted = byteslen;
	}
	for (i = 0; i < byteslen_adjusted + 1; i++) {
		if (!(i % 8)) {
			if (i > 0) {
				for (j = i - 8; j < i; j++) {
					if (j >= byteslen_adjusted) {
						printf("  ");
					} else if (j >= byteslen) {
						printf("  ");
					} else if (bytes[j] >= '!' && bytes[j] <= '~') {
						printf(" %c", bytes[j]);
					} else {
						printf(" .");
					}
				}
			}
			if (i < byteslen_adjusted) {
				printf("\n%02X: ", i);
			}
		} else if (!(i % 4)) {
			printf(" ");
		}
		if (i >= byteslen_adjusted) {
			continue;
		} else if (i >= byteslen) {
			printf("   ");
		} else {
			printf("%02X ", bytes[i]);
		}
	}
	printf("\n");
	fflush(stdout);
}
