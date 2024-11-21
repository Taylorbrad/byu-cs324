#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

#include "sockhelper.h"

/* Recommended max object size */
#define MAX_OBJECT_SIZE 102400

// static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";

int complete_request_received(char *);
void parse_request(char *, char *, char *, char *, char *);
void test_parser();
void print_bytes(unsigned char *, int);
// int open_sfd(char *, char *, struct sockaddr *, socklen_t);

int open_sfd(char *);
void handle_client(int);
int create_modified_request(char *newRequest, char* method, char *hostname, char *port, char *path);
void * threadFunction(void *);
void producer_thread(int);
void *consumer_thread(void *);
void initialize_shared_buffer();

const int THREAD_POOL_SIZE = 8;
const int BUFFER_SIZE = 5;

typedef struct {
	int *buffer;
	int in;
	int out;
	sem_t empty;
	sem_t full;
	pthread_mutex_t mutex;
} shared_buffer_t;

shared_buffer_t shared_buffer;

int main(int argc, char *argv[])
{
	// printf("main 1");
	// char method[16], hostname[64], port[8], path[64];

	// fflush(stdout);
	// printf("%s\n", user_agent_hdr);
	// test_parser();
	// parse_request("GET http://www.example.com/index.html HTTP/1.0", method, hostname, port, path);
	// printf("method: %s\nhostname: %s\nport: %s\npath: %s\n", method, hostname, port, path);

	// socklen_t addr_len;
	// struct sockaddr_storage remote_addr_ss;
	// struct sockaddr *remote_addr = (struct sockaddr *)&remote_addr_ss;

	// int sfd = open_sfd("localhost", argv[1], remote_addr, addr_len);
	int sfd = open_sfd(argv[1]);


	// printf("%s\n", newRequest);
	// pthread_t threads[8];  // Array to store thread identifiers


	// int curThread = 0;

	// while(1) {
	// 	// Threaded
	// 	// int clientfd = accept(sfd, remote_addr, &addr_len);
	// 	//
	// 	// if (pthread_create(&threads[curThread], NULL, threadFunction, (void *)clientfd) != 0) {
	// 	// 	perror("pthread_create failed");
	// 	// 	return 1;
	// 	// }
	// 	// curThread++;
	//
	// 	//Sequential
	// 	// int clientfd = accept(sfd, remote_addr, &addr_len);
	// 	// handle_client(clientfd);
	// }


	// Thread pool
	initialize_shared_buffer();
	 // printf("main");
	 	pthread_t consumer_threads[THREAD_POOL_SIZE];
	 	for (int i = 0; i < THREAD_POOL_SIZE; i++) {
	 		// printf("make %d", i);
	 		fflush(stdout);
	 		pthread_create(&consumer_threads[i], NULL, consumer_thread, NULL);
	 	}
	 	producer_thread(sfd);




		// handle_client((int)clientfd);


	return 0;
}

// int main(int argc, char *argv[]) {
// 	//    test_parser();
// 	printf("%s\n", user_agent_hdr);
//
// 	int port = atoi(argv[1]);
// 	int server_fd = open_sfd(port);
//
//
//
// 	return 0;
// }

void initialize_shared_buffer() {
	shared_buffer.buffer = malloc(BUFFER_SIZE * sizeof(int));
	shared_buffer.in = 0;
	shared_buffer.out = 0;
	sem_init(&shared_buffer.empty, 0, BUFFER_SIZE);
	sem_init(&shared_buffer.full, 0, 0);
	pthread_mutex_init(&shared_buffer.mutex, NULL);
}

void *consumer_thread(void *arg) {
	while (1) {
		int client_fd;

		sem_wait(&shared_buffer.full);
		pthread_mutex_lock(&shared_buffer.mutex);

		client_fd = shared_buffer.buffer[shared_buffer.out];
		shared_buffer.out = (shared_buffer.out + 1) % BUFFER_SIZE;

		pthread_mutex_unlock(&shared_buffer.mutex);
		sem_post(&shared_buffer.empty);

		handle_client(client_fd);
		close(client_fd);
	}
	return NULL;
}

void producer_thread(int server_fd) {
	// printf("prod");
	// fflush(stdout);
	while (1) {
		struct sockaddr_storage client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0) {
			perror("accept");
			continue;
		}
		// printf("accept");
		// fflush(stdout);

		sem_wait(&shared_buffer.empty);
		pthread_mutex_lock(&shared_buffer.mutex);

		shared_buffer.buffer[shared_buffer.in] = client_fd;
		shared_buffer.in = (shared_buffer.in + 1) % BUFFER_SIZE;

		pthread_mutex_unlock(&shared_buffer.mutex);
		sem_post(&shared_buffer.full);
	}
}

void * threadFunction(void * arg) {

	int clientfd = *((int*) arg);

	// free(arg);

	printf("%d", (int)clientfd);
	pthread_detach(pthread_self());

	handle_client((int)clientfd);

	close((int)clientfd);
	return 0;
}

void handle_client(int clientfd) {
	// printf("handle");
	// fflush(stdout);
	char buf[MAX_OBJECT_SIZE];
	char method[16], hostname[64], port[8], path[64];

	// Read from socket until entire request is received
	// int total_bytes_read = 0;
	int bytesRead = 0;
	// int reads = 1;

	// while (reads != 0) {
	// 	reads = read(clientfd, buf + bytesRead, sizeof(buf) - 1 - bytesRead);
	// 	bytesRead += reads;
	// }



	//Old
	bytesRead = read(clientfd, buf, 1023);

	//New
	// while (1) {
	// 	bytesRead = read(clientfd, buf + total_bytes_read,
	// 					  MAX_OBJECT_SIZE - total_bytes_read - 1);
	// 	if (bytesRead <= 0) {
	// 		if (bytesRead == 0) break;  // Connection closed
	// 		perror("read");
	// 		return;
	// 	}
	//
	// 	total_bytes_read += bytesRead;
	// 	buf[total_bytes_read] = '\0';
	//
	// 	if (strstr(buf, "\r\n\r\n")) {
	// 		// Full request received
	// 		break;
	// 	}
	// }

	// Print the request
	print_bytes((unsigned char *)buf, bytesRead);

	// Add null terminator and pass to parse_request
	buf[bytesRead] = '\0';
	parse_request(buf, method, hostname, port, path);

	// Print out components of request
	printf("method: %s\nhostname: %s\nport: %s\npath: %s\n", method, hostname, port, path);

	 char newRequest[1024];
	int requestSize = create_modified_request(newRequest, method, hostname, port, path);
	print_bytes((unsigned char *)newRequest, requestSize);
	// printf("print request");
	// fflush(stdout);
	// create_modified_request(newRequest, method, hostname, port, path);

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPV4
	hints.ai_socktype = SOCK_STREAM; // Use TCP

	getaddrinfo(hostname, port, &hints, &res);

	// Create socket for server conneciton
	int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// Connect to server
	if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
		perror("Connection failed");
		freeaddrinfo(res); // Clean up
		close(sock);
		// return 1;
	}
	freeaddrinfo(res); // Free the address info
	// close(clientfd);
	send(sock, newRequest, requestSize, 0);

	// Receive response
	unsigned char response[16384];
	int bytes_received = 0;
	int received = 1;

	while (received != 0) {
		received = read(sock, response + bytes_received, sizeof(response) - 1 - bytes_received);
		bytes_received += received;
	}

	if (bytes_received > 0) {
		// response[bytes_received] = '\0'; // Null-terminate the response
		print_bytes(response, bytes_received);
	}

	// Clean up
	close(sock);


	// Send response back to client and close connection
	send(clientfd, response, bytes_received, 0);
	close(clientfd);

}

int create_modified_request(char *newRequest, char* method, char *hostname, char *port, char *path) {
	// char newRequest[1024];
	int bytes_written = 0;

	bytes_written += snprintf(newRequest + bytes_written, 1024 - bytes_written, "%s %s HTTP/1.0\r\n", method, path);

	if (strcmp(port, "80") == 0) {
		bytes_written += snprintf(newRequest + bytes_written, 1024 - bytes_written, "Host: %s\r\n", hostname);
	}
	else {
		bytes_written += snprintf(newRequest + bytes_written, 1024 - bytes_written, "Host: %s:%s\r\n", hostname, port);
	}

	bytes_written += snprintf(newRequest + bytes_written, 1024 - bytes_written, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n");

	// newRequest[bytes_written] = '\0';
	// printf("buf: %s", newRequest);
	// return newRequest;
	return bytes_written;
}

// int open_sfd(char *hostname, char *port, struct sockaddr *remote_addr, socklen_t addr_len) {
int open_sfd(char *port) {
	struct addrinfo hints;
	struct addrinfo *result;

	// Initialize everything to 0
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	// Use type SOCK_DGRAM (UDP)
	hints.ai_socktype = SOCK_STREAM;


	// int s;

	getaddrinfo(NULL, port, &hints, &result);

	// int addr_fam;
	// socklen_t addr_len; //**Moved to main


	// char local_ip[INET6_ADDRSTRLEN];
	// unsigned short local_port;

	// See notes above for local_addr_ss and local_addr_ss.
	// struct sockaddr_storage remote_addr_ss; //**Moved to main
	// struct sockaddr *remote_addr = (struct sockaddr *)&remote_addr_ss; //**Moved to main
	// char remote_ip[INET6_ADDRSTRLEN];
	// unsigned short remote_port;


	struct addrinfo *rp;
	int sfd = 1;


	for (rp = result; rp != NULL; rp = rp->ai_next) {
		// Loop through every entry in the linked list populated by
		// getaddrinfo().   If socket(2) (or connect(2)) fails, we
		// close the socket and try the next address.
		//
		// For each iteration of the loop, rp points to an instance of
		// struct * addrinfo that contains the following members:
		//
		// ai_family: The address family for the given address. This is
		//         either AF_INET (IPv4) or AF_INET6 (IPv6).
		// ai_socktype: The type of socket.  This is either SOCK_DGRAM
		//         or SOCK_STREAM.
		// ai_addrlen: The length of the structure used to hold the
		//         address (different for AF_INET and AF_INET6).
		// ai_addr: The struct sockaddr_storage that holds the IPv4 or
		//         IPv6 address and port.

		if ((sfd = socket(rp->ai_family, rp->ai_socktype, 0)) < 0) {
			perror("Error creating socket");
			exit(EXIT_FAILURE);
		}
		// sfd = socket(rp->ai_family, rp->ai_socktype, 0);
		if (sfd < 0) {
			// error creating the socket
			continue;
		}

		// addr_fam = rp->ai_family;
		// addr_len = rp->ai_addrlen;
		// Copy the value of rp->ai_addr to the struct sockaddr_storage
		// pointed to by remote_addr.
		// memcpy(remote_addr, rp->ai_addr, sizeof(struct sockaddr_storage));

		// Extract the remote IP address and port from remote_addr
		// using parse_sockaddr().  parse_sockaddr() is defined in
		// ../code/sockhelper.c.
		// parse_sockaddr(remote_addr, remote_ip, &remote_port);
		// fprintf(stderr, "Connecting to %s:%d (addr family: %d)\n", remote_ip, remote_port, addr_fam);

		// If connect() succeeds, then break out of the loop; we will
		// use the current address as our remote address.
		// if (connect(sfd, remote_addr, addr_len) >= 0)
		// 	break;

		// close(sfd);
		// printf("socket: %d\n", sfd);
		int optval = 1;
		setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) < 0) {
			perror("Could not bind");
			exit(EXIT_FAILURE);
		}
	}
	listen(sfd, 100);

	return sfd;
	// int sfd = socket(rp->ai_family, rp->ai_socktype, 0);
}

int complete_request_received(char *request) {
	// printf("%s", strstr(request, "\r\n\r\n"));

	if (strstr(request, "\r\n\r\n") != NULL) {
		return 1;
	}

	return 0;
}

void parse_request(char *request, char *method, char *hostname, char *port, char *path) {

	long n, strBeginningOffset;

	char firstLine[100];
	char *search = strstr(request, "\n"); // Search for first space to get method
	n = search - request;
	strncpy(firstLine, request, n);
	firstLine[n] = '\0';
	// printf("firstline: %s", firstLine);

	//get and set Method


	// char *searchedString = strstr(request, " "); // Search for first space to get method
	// n = searchedString - request;
	// strncpy(method, request, n);
	// method[n] = '\0';
	char *searchedString = strstr(firstLine, " "); // Search for first space to get method
	n = searchedString - firstLine;
	strncpy(method, firstLine, n);
	method[n] = '\0';

	//get and set Hostname

	// searchedString = strstr(request, "://"); // Search for :// to get beginning of hostname
	// strBeginningOffset = searchedString - request + 3;
	searchedString = strstr(firstLine, "://"); // Search for :// to get beginning of hostname
	strBeginningOffset = searchedString - firstLine + 3;

	// n = strstr(&request[strBeginningOffset], ":") - &request[strBeginningOffset];
	// strncpy(hostname, &request[strBeginningOffset], n);
	// hostname[n] = '\0';

	//get and set port

	// searchedString = strstr(&request[strBeginningOffset], ":"); // Search after the previous search for colon, to get port
	searchedString = strstr(&firstLine[strBeginningOffset], ":"); // Search after the previous search for colon, to get port


	// printf("\nsearch: %s\n", searchedString);

	if (searchedString != NULL) {

		//hostname
		// n = strstr(&request[strBeginningOffset], ":") - &request[strBeginningOffset];
		// strncpy(hostname, &request[strBeginningOffset], n);
		// hostname[n] = '\0';
		n = strstr(&firstLine[strBeginningOffset], ":") - &firstLine[strBeginningOffset];
		strncpy(hostname, &firstLine[strBeginningOffset], n);
		hostname[n] = '\0';

		//port
		// strBeginningOffset = searchedString - request + 1;
		// n = strstr(&request[strBeginningOffset], "/") - &request[strBeginningOffset];
		// strncpy(port, &request[strBeginningOffset], n);
		strBeginningOffset = searchedString - firstLine + 1;
		n = strstr(&firstLine[strBeginningOffset], "/") - &firstLine[strBeginningOffset];
		strncpy(port, &firstLine[strBeginningOffset], n);

		port[n] = '\0';
	}
	else {
		//hostname
		// n = strstr(&request[strBeginningOffset], "/") - &request[strBeginningOffset];
		// strncpy(hostname, &request[strBeginningOffset], n);
		// hostname[n] = '\0';
		n = strstr(&firstLine[strBeginningOffset], "/") - &firstLine[strBeginningOffset];
		strncpy(hostname, &firstLine[strBeginningOffset], n);
		hostname[n] = '\0';

		port[0] = '8';
		port[1] = '0';
		port[2] = '\0';
	}

	//get and set path

	// searchedString = strstr(&request[strBeginningOffset], "/");
	// strBeginningOffset = searchedString - request;
	// n = strstr(&request[strBeginningOffset], " ") - &request[strBeginningOffset];
	searchedString = strstr(&firstLine[strBeginningOffset], "/");
	strBeginningOffset = searchedString - firstLine;
	n = strstr(&firstLine[strBeginningOffset], " ") - &firstLine[strBeginningOffset];


	strncpy(path, searchedString, n);
	path[n] = '\0';
	// printf("%s", path[strlen(path)]);
}

void test_parser() {
	int i;
	char method[16], hostname[64], port[8], path[64];

       	char *reqs[] = {
		"GET http://www.example.com/index.html HTTP/1.0\r\n"
		"Host: www.example.com\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html?foo=1&bar=2 HTTP/1.0\r\n"
		"Host: www.example.com:8080\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://localhost:1234/home.html HTTP/1.0\r\n"
		"Host: localhost:1234\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html HTTP/1.0\r\n",

		NULL
	};
	
	for (i = 0; reqs[i] != NULL; i++) {
		printf("Testing \n%s\n", reqs[i]);
		if (complete_request_received(reqs[i])) {
			printf("REQUEST COMPLETE:\n");
			parse_request(reqs[i], method, hostname, port, path);
			printf("METHOD: %s\n", method);
			printf("HOSTNAME: %s\n", hostname);
			printf("PORT: %s\n", port);
			printf("PATH: %s\n\n", path);
		} else {
			printf("REQUEST INCOMPLETE\n");
		}
	}
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
