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

void delimiterFinder(char* buffer)
{
    char delimiter = ',';
    char replacement = '\07';

    char* charPtr = buffer;
    int i = 0;

    while (*charPtr != '\0')
    {
        if (*charPtr == delimiter)
        {
            buffer[i] = replacement;
            // cout << "DEBUG - replaced at - " << i << endl;
        }

        charPtr++;
        i++;
    }
}

char* createMessage(char* authorBuffer, char* msgBuffer)
{
    // Get topic and message body from the user
    char topicBuffer[64] = {};
    cout << "Topic: ";
    cin.getline(topicBuffer, 64);

    if (topicBuffer[0] == '\0')
    {
        strcpy(topicBuffer, "none");
    }

    char bodyBuffer[512] = { "\0" };
    cout << "Body: ";
    cin.getline(bodyBuffer, 512);

    delimiterFinder(topicBuffer);
    delimiterFinder(bodyBuffer);


    // Create combined message to send to server
    strcpy(msgBuffer, topicBuffer);
    strcat(msgBuffer, ",");          // Seperate sections with comma
    strcat(msgBuffer, authorBuffer);
    strcat(msgBuffer, ",");
    strcat(msgBuffer, bodyBuffer);
    cout << "DEBUG - Message: " << msgBuffer << endl;

    return msgBuffer;
}

void intMessageConversion(int i, SOCKET socket)
{
    char intString[16];
    sprintf(intString, "%d", i);

    send(socket, intString, strlen(intString), 0);
}

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
    SvrAddr.sin_addr.s_addr = inet_addr("172.16.5.12"); // Server address

    if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR)
    {
        close(ClientSocket);
        cout << "ERROR: Connection attempt failed" << endl;

        return 0;
    }

    cout << "Connected to server" << endl;
    bool talking = true;

    // Set up the author 1 time
    char authorBuffer[64] = { "\0" };
    cout << "Enter name here (or press Enter for anonymous): ";
    cin.getline(authorBuffer, 64); // Get line lets us detect "Enter" presses (rather than just cin >>)

    if (authorBuffer[0] == '\0')
    {
        strcpy(authorBuffer, "anonymous");
    }

    cout << "Name is: " << authorBuffer << endl; // DEBUG


    // Recive the servers welcome message
    char welcomeBuffer[64] = { 0 };
    recv(ClientSocket, welcomeBuffer, sizeof(welcomeBuffer), 0);
    cout << welcomeBuffer << endl;

    // Send/Recv message loop
    while (talking)
    {
        int optionVal;
        cout << "Option (quit - 0, write - 1, read - 2): ";
        cin >> optionVal;

        intMessageConversion(optionVal, ClientSocket);

        switch (optionVal)
        {
        case 0: // Quit
        {
            talking = false;
            break;
        }

        case 1: // Write
        {
            int messageCount;
            cout << "How many messages are in your collection? ";
            cin >> messageCount;

            intMessageConversion(messageCount, ClientSocket);

            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Igonre the latest int input 
            for (int i = 0; i < messageCount; i++)
            {
                char msgBuffer[640];
                createMessage(authorBuffer, msgBuffer);

                // Send message (Topic - Author - Body)
                send(ClientSocket, msgBuffer, strlen(msgBuffer), 0);
            }
            break;
        }

        case 2: // Read
        {
            char rcBuffer[64] = { 0 };
            int bytesReceived = recv(ClientSocket, rcBuffer, sizeof(rcBuffer), 0);

            rcBuffer[bytesReceived] = '\0';
            int readCount = atoi(rcBuffer);

            cout << "Reading " << readCount << " messages..." << endl;

            for (int i = 0; i < readCount; i++)
            {
                char rcBuffer[640] = { 0 };

                // Recv message from server (Topic - Author - Body)
                int bytesReceived = recv(ClientSocket, rcBuffer, sizeof(rcBuffer), 0);

                if (bytesReceived > 0)
                {
                    rcBuffer[bytesReceived] = '\0'; // Null terminate
                    cout << "Message " << (i + 1) << ": " << rcBuffer << endl;
                }

                else if (bytesReceived == 0)
                {
                    cout << "Server disconnected" << endl;
                    return 0;
                }

                else
                {
                    cout << "ERROR: recv failed on message " << (i + 1) << endl;
                    return 0;
                }
            }

            break;
        }

        default:
        {
            break;
        }
        }
    }

    // Clean up the sockets at the end
    close(ClientSocket);

    return 0;
}
