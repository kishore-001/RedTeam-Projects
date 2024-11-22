/* This is threaded client which will interact with the server....*/

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <threads.h>
#include <unistd.h>

using namespace std;

#define PORT 1111
#define MAXCLIENT 10

struct sockaddr_in serverinfo;

void clienthandle(int clientfd) {
  // Main loop to monitor the socket for incoming data
  fd_set readfds;         // File descriptor set for `select()`
  struct timeval timeout; // Timeout for `select()`

  while (true) {
    // Clear and set the file descriptor set
    FD_ZERO(&readfds);
    FD_SET(clientfd, &readfds);

    // Set a timeout to prevent indefinite blocking
    timeout.tv_sec = 0;       // Zero seconds
    timeout.tv_usec = 500000; // Half a second

    // Check the socket for incoming data
    int activity = select(clientfd + 1, &readfds, NULL, NULL, &timeout);

    if (activity < 0) {
      cerr << "Error in select(). Exiting." << endl;
      break; // Exit on failure
    }

    if (activity > 0 && FD_ISSET(clientfd, &readfds)) {
      // Socket has data to read
      char buffer[1024] = {0};
      int valread = read(clientfd, buffer, sizeof(buffer));

      if (valread < 0) {
        cerr << "Error reading data from the server." << endl;
        break; // Exit on failure
      } else if (valread == 0) {
        cout << "Disconnected from the server." << endl;
        exit(EXIT_FAILURE);
        break; // Exit on server disconnection
      } else {
        // Print the received message
        cout << endl << "Message from the server: " << buffer << endl;
      }
    }

    // Non-blocking loop continues even if no data is available
    // Perform other tasks here if needed
  }
}

int main() {
  int clientsocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientsocket < 0) {
    cout << "Failed to create a socket ..." << endl;
    close(clientsocket);
    return EXIT_FAILURE;
  }

  serverinfo.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverinfo.sin_port = htons(PORT);
  serverinfo.sin_family = AF_INET;
  memset(&serverinfo.sin_zero, 0, 8);

  if (connect(clientsocket, (struct sockaddr *)&serverinfo,
              sizeof(serverinfo)) < 0) {
    cout << "Cannot communicate to the server .." << endl;
    close(clientsocket);
    return EXIT_FAILURE;
  }
  cout << "Connected to the server successfully." << endl;

  thread client_thread(clienthandle, clientsocket);

  string message;
  while (true) {
    cout << "Enter the message your are going to sent to the server ... : ";
    getline(cin, message);

    if (message == "exit") {
      cout << "Exiting ...." << endl;
      break;
    }

    if (send(clientsocket, message.c_str(), message.size(), 0) < 0) {
      cout << "Failed to sent the data" << endl;
      break;
    }
  }
  close(clientsocket);
  client_thread.join();
  return 0;
}
