#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define SOCKET int
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1


int main(void) {

	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET) {
		printf("ERROR: Failed to create ClientSocket");
		return 0;
	}
	printf("successful creation of ClientSocket\n");

	sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(27000);
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
		close(ClientSocket);
		//WSACleanup();
		printf("ERROR: Failed to connect");
		return 0;
	}
	printf("successful Connection\n");

	char TxBuffer[50] = {};
	char RxBuffer[50] = {};
	bool getInput = true;

	while (getInput) {

		printf("Enter A String\n");
		fgets(TxBuffer, sizeof(TxBuffer), stdin);
		send(ClientSocket, TxBuffer, static_cast<int>(strlen(TxBuffer)), 0);

		//recv(ClientSocket, RxBuffer, sizeof(RxBuffer), 0);
		int n = recv(ClientSocket, RxBuffer, static_cast<int>(sizeof(RxBuffer) - 1), 0);
		if (n > 0) {
			RxBuffer[n] = '\0';
			printf("Rx: %s\n", RxBuffer);
		}
		else if (n == 0) {
			printf("Server closed connection\n");
			break;
		}
		else {
			printf("recv failed\n");
			break;
		}
		//printf("Rx: %s", RxBuffer);

		if (RxBuffer[0] == 'x')
			break;
	}

	close(ClientSocket);
	//WSACleanup();



	return 0;
}