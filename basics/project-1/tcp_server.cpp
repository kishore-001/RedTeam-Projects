
/* This is a TCP server which has the ability to handle multiple client upto 10
 * as per this code this receive the message from the client and send it to the
 * sever... */

#include <arpa/inet.h>  // inet_addr()
#include <cstdio>       // perror()
#include <cstdlib>      // EXIT_FAILURE
#include <cstring>      // memset()
#include <iostream>     // cout
#include <netinet/in.h> // sockaddr_in structure
#include <sys/select.h> // select()
#include <sys/socket.h> // socket(),bind(),accept(),send(),recv() and so on ...
#include <sys/types.h>  //size_t
#include <unistd.h>     //close()

using namespace std;

// Declaring the constant

#define PORT 11111
#define MAXCLIENT 10

// structure used to store the information about the IP address, PORT address
// and So on...

struct sockaddr_in srv;

int main() {

  // SOCKET CREATION ...
  int socket1 = socket(AF_INET, SOCK_STREAM, 0);
  if (socket1 < 0) {
    cout << "Failed to create the socket" << endl;
    close(socket1);
    return EXIT_FAILURE;
  }
  cout << "Socket created successfully" << endl;

  // SOCKADDR STRUCTURE CREATION
  srv.sin_family = AF_INET;
  srv.sin_port = htons(PORT);
  srv.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(&srv.sin_zero, 0, 8);

  // SOCKET OPTIONS
  int optval = 1;
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
      0) {
    perror("setsockopt failed");
    close(socket1);
    return EXIT_FAILURE;
  }

  // BINDING THE SOCKET WITH THE PORT
  if (bind(socket1, (sockaddr *)&srv, sizeof(srv)) < 0) {
    cout << "Failed to bind the socket to the port" << endl;
    close(socket1);
    return EXIT_FAILURE;
  }
  cout << "Successfully binded to the socket" << endl;

  // LISTENING TO THE SOCKET
  if (listen(socket1, 5) < 0) {
    perror("Listen failed");
    close(socket1);
    return EXIT_FAILURE;
  }

  // fd_set contains the socket and file descriptor to check whether they are
  // active ...

  fd_set fr;
  int maxfd = socket1;
  int clientarray[MAXCLIENT] = {
      0}; // Initialize all elements to 0 (no clients connected)

  while (1) {
    FD_ZERO(&fr);
    FD_SET(socket1, &fr);
    timeval time;
    time.tv_sec = 1;
    time.tv_usec = 0;

    for (int i = 0; i < MAXCLIENT; i++) {
      if (clientarray[i] != 0) {
        FD_SET(clientarray[i], &fr);
        if (clientarray[i] > maxfd) {
          maxfd = clientarray[i];
        }
      }
    }

    int res = select(maxfd + 1, &fr, NULL, NULL, &time);

    if (res > 0) {

      /* check whether the master socket(server socket) is active ...
       * in the below if condition is yes it handle the clientsocket and
       * add it the clientsocket array ..*/

      if (FD_ISSET(socket1, &fr)) {
        cout << "Handling new client connection" << endl;
        int clientsocket = accept(socket1, NULL, NULL);
        if (clientsocket < 0) {
          perror("Failed to Accept");
        } else {
          cout << "Client Socket Connected: " << clientsocket << endl;
          bool added = false;
          for (int i = 0; i < MAXCLIENT; i++) {
            if (clientarray[i] == 0) {
              clientarray[i] = clientsocket;
              added = true;
              break;
            }
          }
          if (!added) {
            cout << "Max Clients reached, rejecting connection" << endl;
            close(clientsocket);
          }
        }
      }

      /* This for loop checks whether the clientsocket is active using the
       * FD_ISSET() funciton ..*/

      for (int i = 0; i < MAXCLIENT; i++) {
        if (clientarray[i] != 0 && FD_ISSET(clientarray[i], &fr)) {
          char buffer[1024] = {0};
          int valread = read(clientarray[i], buffer, sizeof(buffer));
          if (valread <= 0) {
            cout << "Client Disconnected: " << clientarray[i] << endl;
            close(clientarray[i]);
            clientarray[i] = 0;
          } else {
            cout << "Received from client " << clientarray[i] << ": " << buffer
                 << endl;
          }
        }
      }
    } else if (res == 0) {
      cout << "No active file descriptor" << endl;
      continue;
    } else {
      cout << "Error occurred in select()" << endl;
      break;
    }
  }

  // Close all open client sockets
  for (int i = 0; i < MAXCLIENT; i++) {
    if (clientarray[i] != 0) {
      close(clientarray[i]);
    }
  }
  close(socket1);

  return 0;
}
