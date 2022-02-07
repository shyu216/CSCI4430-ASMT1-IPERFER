#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <time.h>

int dataSize = 1000;
char *fin = (char *)malloc(dataSize);
char *data = (char *)malloc(dataSize);

void init()
{
	memset(data, '0', dataSize);
	memset(fin, '1', dataSize);
}

int make_server_sockaddr(struct sockaddr_in *addr, int port)
{

	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_port = htons((unsigned short int)port);

	return 0;
}

int make_client_sockaddr(struct sockaddr_in *addr, const char *hostname, int port)
{

	addr->sin_family = AF_INET;
	struct hostent *host = gethostbyname(hostname);
	if (host == nullptr)
	{
		//fprintf(stderr, "%s: unknown host\n", hostname);
		exit(0);
	}
	memcpy(&(addr->sin_addr), host->h_addr, host->h_length);
	addr->sin_port = htons((unsigned short int)port);

	return 0;
}

int server(int port)
{

	// (1) Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		//perror("Error opening stream socket");
		exit(0);
	}

	// (2) Set the "reuse port" socket option
	int yesval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1)
	{
		//perror("Error setting socket options");
		exit(0);
	}

	// (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
	if (make_server_sockaddr(&addr, port) == -1)
	{
		exit(0);
	}

	// (3b) Bind to the port.
	if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		//perror("Error binding stream socket");
		exit(0);
	}

	// (4) Begin listening for incoming connections.
	int queue_size = 1000;
	if (listen(sockfd, queue_size) == -1)
	{
		//perror("Error listening connection");
		exit(0);
	}

	// (5) Server function

	// (5a) Keep receiving all data
	int connectionfd = accept(sockfd, 0, 0);
	if (connectionfd == -1)
	{
		//perror("Error accepting connection");
		exit(0);
	}
	// printf("connect: %d, sock: %d\n", connectionfd, sockfd);

	long int byteReceived = 0;
	time_t start = time(NULL);
	ssize_t rval;

	do
	{
		rval = recv(connectionfd, data, dataSize, 0);
		if (rval == -1)
		{
			//perror("Error reading stream message");
			exit(0);
		}
		byteReceived += (int)rval;
		// printf("rece %d, total %ld, data %c\n", (int)rval, byteReceived, data[0]);

		// (5b) Catch FIN and send ACK
		if (byteReceived % dataSize == 0 && data[0] == fin[0])
		{
			// printf("receive fin:%c\n", data[0]);
			if (send(connectionfd, fin, dataSize, 0) == -1)
			{
				//perror("Error sending stream message");
				exit(0);
			}
			break;
		}
	} while (rval > 0);

	// (6) Close connection
	shutdown(connectionfd, 2);
	time_t end = time(NULL);

	// (7) Print summary
	// printf("difftime:%f\n", difftime(end, start));
	printf("Received=%ld KB, Rate=%.3f Mbps\n", byteReceived / 1000, 8.0 * (double)byteReceived / difftime(end, start) / 1000000);

	// (8) Shutdown server
	shutdown(sockfd, 2);

	return 0;
}

int client(const char *hostname, int port, int duration)
{

	// (1) Create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		//perror("Error opening stream socket");
		exit(0);
	}

	// (2) Create a sockaddr_in to specify remote host and port
	struct sockaddr_in addr;
	if (make_client_sockaddr(&addr, hostname, port) == -1)
	{
		exit(0);
	}

	// (3) Connect to remote server
	if (connect(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		//perror("Error connecting stream socket");
		exit(0);
	}

	// (4) Client function
	// printf("connect: %d\n",sockfd);

	// (4a) Keep sending for the duration
	long int byteSent = 0;
	time_t start = time(NULL);
	time_t end;
	double d = 0.0 + duration;

	do
	{
		if (send(sockfd, data, dataSize, 0) == -1)
		{
			//perror("Error sending stream message");
			exit(0);
		}
		byteSent += dataSize;
		// printf("send %d, total %ld, data %c\n", dataSize, byteSent, data[0]);
		end = time(NULL);
	} while (d > difftime(end, start));

	// (4b) Send FIN and Recieve ACK
	if (send(sockfd, fin, dataSize, 0) == -1)
	{
		//perror("Error sending stream message");
		exit(0);
	}
	byteSent += dataSize;
	// printf("sent %d, total %ld, data %c\n", dataSize, byteSent, fin[0]);
	char *ack = (char *)malloc(dataSize);
	int ackByte = 0;
	ssize_t rval;
	do
	{
		rval = recv(sockfd, ack, dataSize, 0);
		if (rval == -1)
		{
			//perror("Error reading stream message");
			exit(0);
		}
		ackByte += (int)rval;
	} while (ackByte < dataSize);
	// printf("receive ack:%c len:%d\n", ack[0], (int)ackByte);
	end = time(NULL);

	// (5) Close connection
	shutdown(sockfd, 2);

	// (6) Print summary
	// printf("difftime:%f\n", difftime(end, start));
	printf("Sent=%ld KB, Rate=%.3f Mbps\n", byteSent / 1000, 8.0 * (double)byteSent / difftime(end, start) / 1000000);

	return 0;
}

int main(int argc, const char **argv)
{
	// printf("%d\n",argc);
	init();
	if (argc < 2)
	{
		printf("Error: missing or extra arguments\n");
		return 1;
	}
	else if (strcmp(argv[1], "-s") == 0)
	{
		if (argc != 4 || strcmp(argv[2], "-p") != 0)
		{
			printf("Error: missing or extra arguments\n");
			return 1;
		}
		int port = atoi(argv[3]);
		if (port < 1024 || port > 65535)
		{
			printf("Error: port number must be in the range of [1024, 65535]\n");
			return 1;
		}
		if (server(port) == -1)
		{
			return 1;
		}
		exit(0);
	}
	else if (strcmp(argv[1], "-c") == 0)
	{
		if (argc != 8)
		{
			printf("Error: missing or extra arguments\n");
			return 1;
		}
		if (strcmp(argv[2], "-h") != 0 || strcmp(argv[4], "-p") != 0 || strcmp(argv[6], "-t") != 0)
		{
			printf("Error: missing or extra arguments\n");
			return 1;
		}
		int port = atoi(argv[5]);
		if (port < 1024 || port > 65535)
		{
			printf("Error: port number must be in the range of [1024, 65535]\n");
			return 1;
		}
		int duration = atoi(argv[7]);
		if (duration <= 0)
		{
			printf("Error: time argument must be greater than 0\n");
			return 1;
		}
		const char *host = argv[3];
		if (client(host, port, duration) == -1)
		{
			return 1;
		}
		exit(0);
	}
	printf("Error: missing or extra arguments\n");
	return 1;
}
