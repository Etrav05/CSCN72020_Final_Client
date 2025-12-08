#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define INVALID_SOCKET -1
#define SOCKET int
#define SOCKET_ERROR -1

using namespace std;

int main() {
    // Create a TCP server socket
    SOCKET ServerSocket;
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Changed to SOCK_STREAM for TCP
    if (ServerSocket < 0) {
        cout << "ERROR: Failed to create socket\n";
        return 0;
    }

    // Bind socket to port
    sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr.s_addr = INADDR_ANY;
    SvrAddr.sin_port = htons(27000);

    if (bind(ServerSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
        close(ServerSocket);
        cout << "ERROR: Failed to bind ServerSocket" << endl;
        return 0;
    }

    // Set up server listening
    if (listen(ServerSocket, 1) == SOCKET_ERROR) {
        close(ServerSocket);
        cout << "ERROR: Listening failed to configure ServerSocket" << endl;
        return 0;
    }
    cout << "Server waiting for connection" << endl;

    // Accept client connection
    SOCKET ConnectionSocket;
    if ((ConnectionSocket = accept(ServerSocket, NULL, NULL)) == SOCKET_ERROR) {
        close(ServerSocket);
        cout << "ERROR: ConnectionSocket failed to accept ServerSocket" << endl;
        return 0;
    }
    cout << "Client connection complete" << endl;

    // Message loop
    bool connected = true;
    while (connected) {
        // Receive message from client
        char rcBuffer[640] = {};
        int bytes = recv(ConnectionSocket, rcBuffer, sizeof(rcBuffer), 0); // Use recv for TCP

        if (bytes > 0) {
            rcBuffer[bytes] = '\0';
            cout << "Received: " << rcBuffer << endl;

            // Send back a response
            char sdBuffer[128] = "Message received by server!";
            send(ConnectionSocket, sdBuffer, strlen(sdBuffer), 0); // Use send for TCP
        }
        else if (bytes == 0) {
            cout << "Client disconnected" << endl;
            connected = false;
        }
        else {
            cout << "ERROR: recv failed" << endl;
            connected = false;
        }
    }

    // Clean up
    close(ConnectionSocket);
    close(ServerSocket);

    return 0;
}