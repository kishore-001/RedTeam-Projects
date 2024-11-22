/* This is TCP multithreaded chat server ... which will use multiple thread to
 * handle multiple client */

// First let us include the header files....

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// after the header file we need to include the standard namespace

using namespace std;

// declare the constant we are going to use ..

#define PORT 1111
#define MAXCLIENT 10

// declare the struct to store the server info

struct sockaddr_in srv;
int clientarray[MAXCLIENT] = {0};
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcasting(char message[], int client_socket) {
  for (int i = 0; i < MAXCLIENT; i++) {
    if (clientarray[i] != client_socket && clientarray[i] != 0) {
      if (send(clientarray[i], message, strlen(message), 0) < 0) {
        cout << "Failed to send the message ...." << endl;
        continue;
      }
      cout << "The Broadcasted message is : " << message << endl;
    }
  }
}

void *handle_client(void *arg) {
  int client_socket = *(int *)arg;
  free(arg);

  char buffer[1024];
  while (true) {
    int readval = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (readval < 0) {
      cout << "Failed to read the data from the client..." << endl;
    } else if (readval == 0) {
      cout << "Client " << client_socket << " disconnected." << endl;

      // Remove client from the array
      pthread_mutex_lock(&client_mutex);
      for (int i = 0; i < MAXCLIENT; i++) {
        if (clientarray[i] == client_socket) {
          clientarray[i] = 0;
          break;
        }
      }
      pthread_mutex_unlock(&client_mutex);

      close(client_socket);
      return NULL;
    }

    buffer[readval] = '\0'; // Null-terminate received data
    cout << "Received from client " << client_socket << ": " << buffer << endl;

    char message[2048];
    snprintf(message, sizeof(message), "Client %d says: %s", client_socket,
             buffer);

    broadcasting(message, client_socket);
  }
}

// Program starts here  :)

int main() {
  int srvfd = socket(AF_INET, SOCK_STREAM, 0);
  if (srvfd < 0) {
    cout << "Failed to create the server socket ..." << endl;
    close(srvfd);
    return EXIT_FAILURE;
  }
  cout << "Successfully created the server socket ...";

  // after this we need to assign server info to the "struct sockaddr_in" ...

  srv.sin_family = AF_INET;
  srv.sin_addr.s_addr = inet_addr("127.0.0.1");
  srv.sin_port = htons(PORT);
  memset(srv.sin_zero, 0, 8);

  // after this we need to set the socket option...

  int optval = 1;
  if (setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
      0) {
    cout << "Failed to set socket options ..." << endl;
    close(srvfd);
    return EXIT_FAILURE;
  }

  // after setting the socket options we can bind with the port....
  if (bind(srvfd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
    cout << "Failed to bind the socket ..." << endl;
    close(srvfd);
    return EXIT_FAILURE;
  }

  // after start the server to listen to the port ...
  if (listen(srvfd, 5)) {
    cout << "Failed to start listening on the port ..." << endl;
    close(srvfd);
    return EXIT_FAILURE;
  }

  // after this comes the main logic inside the while loop

  fd_set fr;
  int maxfd = srvfd;

  while (true) {
    FD_ZERO(&fr);
    FD_SET(srvfd, &fr);

    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;

    for (int i = 0; i < MAXCLIENT; i++) {
      if (clientarray[i] != 0) {
        FD_SET(clientarray[i], &fr);
        if (maxfd < clientarray[i]) {
          maxfd = clientarray[i];
        }
      }
    }

    int activity = select(maxfd + 1, &fr, NULL, NULL, &time);
    if (activity > 0) {

      // This block handle the client connection and store it to clientarray
      // ....

      if (FD_ISSET(srvfd, &fr)) {
        cout << "Connection detected ..." << endl;
        int clientsocket = accept(srvfd, NULL, NULL);
        if (clientsocket < 0) {
          cout << "Failed to connect to the server ....";
        } else {
          pthread_mutex_lock(&client_mutex);
          bool added = false;
          for (int j = 0; j < MAXCLIENT; j++) {
            if (clientarray[j] == 0) {
              clientarray[j] = clientsocket;
              added = true;
              break;
            }
          }
          pthread_mutex_unlock(&client_mutex);
          if (!added) {
            cout << "Max Number of Client has reached ..." << endl;
          }
        }
      }

      // after this we need to handle the client .. and passing the thread ...
      for (int i = 0; i < MAXCLIENT; i++) {
        if (FD_ISSET(clientarray[i], &fr) > 0) {
          // handle the client by passing it to the function .........
          int *client_socket_ptr = (int *)malloc(sizeof(int));
          *client_socket_ptr = clientarray[i];
          pthread_t thread_id;
          pthread_create(&thread_id, NULL, handle_client,
                         (void *)client_socket_ptr);
          pthread_detach(thread_id);
        }
      }

    } else if (activity == 0) {
      cout << "Till now there is no client is active" << endl;
      usleep(1000000);
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
