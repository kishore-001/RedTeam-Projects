/* This client is used to receive messages that are broadcast
 * by the server in a non-blocking manner. */

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define PORT 11111

struct sockaddr_in srv;

int main() {
  // Create the client socket
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (clientfd < 0) {
    cerr << "Failed to create a client socket." << endl;
    return EXIT_FAILURE;
  }
  cout << "Client socket created successfully." << endl;

  // Initialize the server structure
  srv.sin_family = AF_INET;
  srv.sin_port = htons(PORT);
  srv.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(&srv.sin_zero, 0, sizeof(srv.sin_zero));

  // Connect to the server
  if (connect(clientfd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
    cerr << "Connection to the server failed." << endl;
    close(clientfd);
    return EXIT_FAILURE;
  }
  cout << "Connected to the server successfully." << endl;

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
        break; // Exit on server disconnection
      } else {
        // Print the received message
        cout << "Message from the server: " << buffer << endl;
      }
    }

    // Non-blocking loop continues even if no data is available
    // Perform other tasks here if needed
  }

  // Close the socket when done
  close(clientfd);
  return 0;
}
