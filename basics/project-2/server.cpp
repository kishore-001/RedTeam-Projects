/* This is a TCP server which is client chat server which accept
 * the message and replay it to the other client that are connected
 * to the server */

#include <arpa/inet.h> // inet_addr()
#include <asm-generic/socket.h>
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

struct sockaddr_in addr;

int main() {

  // SOCKET CREATION ...
  int srvfd = socket(AF_INET, SOCK_STREAM, 0);
  if (srvfd < 0) {
    cout << "Failed to create the socket" << endl;
    return EXIT_FAILURE;
  }
  cout << "Socket created successfully" << endl;

  // SOCKADDR STRUCTURE CREATION
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(&addr.sin_zero, 0, 8);

  // after this we need to set the setsockopt() funciton which will
  // be used to set many socket option

  int optval = 1;
  if (setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
      0) {
    cout << "Failed to set the options to the socket ..." << endl;
    close(srvfd);
    return EXIT_FAILURE;
  }

  // after the setting the options we need to bind the socket with port and
  // IP address ...

  if (bind(srvfd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    cout << "Failed to bind the socket to the  IP address and the Port "
         << endl;
    close(srvfd);
    return EXIT_FAILURE;
  }

  // after the binding to the socket we need to listen
  // or start the server using listen() funciton ..

  if (listen(srvfd, 5)) {
    perror("Listening failed ...");
    close(srvfd);
    return EXIT_FAILURE;
  }

  // here after we need implemend the logic which is need for the Server...
  fd_set fr, fw;
  int maxfd = srvfd;
  int clientarray[MAXCLIENT] = {0};

  while (1) {

    FD_ZERO(&fr);
    FD_ZERO(&fw);

    FD_SET(srvfd, &fr);

    timeval time;
    time.tv_sec = 0;
    time.tv_usec = 500000;

    for (int i = 0; i < MAXCLIENT; i++) {
      if (clientarray[i] != 0) {
        FD_SET(clientarray[i], &fr);
        if (clientarray[i] > maxfd) {
          maxfd = clientarray[i];
        }
      }
    }

    int res = select(maxfd + 1, &fr, &fw, NULL, &time);

    if (res > 0) {
      if (FD_ISSET(srvfd, &fr)) {
        cout << "New Connection is detected ..." << endl;
        int clientsocket = accept(srvfd, NULL, NULL);
        if (clientsocket < 0) {
          cout << "Failed to connect to the client" << endl;
        } else {
          cout << "The connected client socket file descriptor.." << endl;
          bool added = false;
          for (int i = 0; i < MAXCLIENT; i++) {
            if (clientarray[i] == 0) {
              clientarray[i] = clientsocket;
              added = true;
              break;
            }
          }
          if (!added) {
            cout << "Max clients have reached , try after a some time" << endl;
          }
        }
      }

      for (int i = 0; i < MAXCLIENT; ++i) {
        if (clientarray[i] != 0 && FD_ISSET(clientarray[i], &fr)) {
          char buffer[1024] = {0};
          int valread = read(clientarray[i], &buffer, sizeof(buffer));
          if (valread < 0) {
            cout << "Failed to read the data form clientsocket : "
                 << clientarray[i] << endl;
          } else if (valread == 0) {
            cout << "Client is disconnected : " << clientarray[i] << endl;
            close(clientarray[i]);
            clientarray[i] = 0;
          } else {
            struct sockaddr_in clientinfo;
            socklen_t clientstructlen = sizeof(clientinfo);
            char client_ip_addr[INET_ADDRSTRLEN];
            int client_port = 0;
            if (getpeername(clientarray[i], (struct sockaddr *)&clientinfo,
                            &clientstructlen) == 0) {
              inet_ntop(AF_INET, &clientinfo.sin_addr, client_ip_addr,
                        sizeof(client_ip_addr));
              client_port = ntohs(clientinfo.sin_port);
            }
            char message[2048]; // Increased buffer size to accommodate the
                                // message
            snprintf(message, sizeof(message),
                     "Message from the IP ADDR: %s PORT: %d\n%s",
                     client_ip_addr, client_port, buffer);
            for (int j = 0; j < MAXCLIENT; j++) {
              if (clientarray[j] != 0 && clientarray[i] != clientarray[j]) {

                int send_val =
                    send(clientarray[j], &message, strlen(message), 0);
                if (send_val < 0) {
                  cout << "Failed to send message to client: " << clientarray[j]
                       << endl;
                  continue;
                }
              }
            }
            cout << message << endl;
          }
        }
      }

    } else if (res == 0) {
      cout << "Till now there is no client is active" << endl;
    } else {
      cout << "Failed to check for the socket" << endl;
      break;
    }
  }

  // after the work we need to close all the client and server socket ..
  for (int i = 0; i < MAXCLIENT; i++) {
    if (clientarray[i] != 0) {
      close(clientarray[i]);
    }
  }

  close(srvfd);

  return 0;
}
