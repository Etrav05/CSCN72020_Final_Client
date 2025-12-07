#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // Included so we can use the inet_addr function below
#include <limits>

#define INVALID_SOCKET -1
#define SOCKET int
#define SOCKET_ERROR -1

using namespace std;

int main()
{
    // Creating client socket
    SOCKET ClientSocket;
    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (ClientSocket < 0)
    {
        cout << "ERROR: Failed to create socket" << endl;
        return 0;
    }

    // Establishing connection
    sockaddr_in SvrAddr{};
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(27000);
    SvrAddr.sin_addr.s_addr = inet_addr("172.16.5.50"); // Server address

    if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR)
    {
        close(ClientSocket);
        cout << "ERROR: Connection attempt failed" << endl;

        return 0;
    }

    cout << "Connected to server" << endl;
    bool talking = true;

    // Set up author 1 time
    char authorBuffer[64] = {};
    cout << "Enter name here (or press Enter for anonymous): ";
    cin.getline(authorBuffer, 64); // Get line lets us detect "Enter" presses (rather than just cin >>)

    if (authorBuffer[0] == '\0') {
        strcpy(authorBuffer, "anonymous");
    }

    cout << "Name is: " << authorBuffer << endl; // DEBUG

    // Send/Recv message loop
    while (talking)
    {
        // Get topic and message body from the user
        char topicBuffer[64] = {};
        char bodyBuffer[512] = {};

        cout << "Topic: ";
        cin.getline(topicBuffer, 64);

        cout << "Body: ";
        cin.getline(bodyBuffer, 512);


        // Create combined message to send to server
        char msgBuffer[640];
        strcpy(msgBuffer, topicBuffer);
        strcat(msgBuffer, ",");          // Seperate sections with comma
        strcat(msgBuffer, authorBuffer);
        strcat(msgBuffer, ",");
        strcat(msgBuffer, bodyBuffer);
        cout << "DEBUG - Message: " << msgBuffer << endl;


        // Send message (Topic - Author - Body)
        send(ClientSocket, msgBuffer, strlen(msgBuffer), 0);


        // Receive response
        char rcBuffer[640] = {};
        int msg = recv(ClientSocket, rcBuffer, sizeof(rcBuffer), 0);

        if (msg > 0) 
        {
            rcBuffer[msg] = '\0';
            cout << "Server response: " << rcBuffer << endl;

            if (strcmp(rcBuffer, "x") == 0) // If the server enters 'x' to leave the conversation
            {
                talking = false;
            }
        }

        else 
        {
            cout << "ERROR: recv failed" << endl;
            talking = false;
        }


        // Ask if user wants to continue
        cout << "Send another message? (y/N): ";
        char choice;
        cin.getline(choice, 5);

        if (choice != 'y' && choice != 'Y') {
            talking = false;
        }
    }

    // Clean up the sockets at the end
    close(ClientSocket);

    return 0;
}
