#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

#define PORT 11111

struct sockaddr_in server;

int main() {
  int clientsocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientsocket < 0) {
    cout << "Failed to create the socket" << endl;
    close(clientsocket);
    return EXIT_FAILURE;
  }
  cout << "Socket is created successfully" << endl;

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = INADDR_ANY;
  memset(&server.sin_zero, 0, 8);

  if (connect(clientsocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
    cerr << "Connection to server failed" << endl;
    close(clientsocket);
    return EXIT_FAILURE;
  }
  cout << " Connected to the server" << endl;
  sockaddr_in client;
  socklen_t client_len = sizeof(client);
  if (getsockname(clientsocket, (sockaddr *)&client, &client_len) == 0) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(client.sin_port);

    std::cout << "Client is using IP: " << client_ip
              << " and Port: " << client_port << std::endl;
  } else {
    perror("getsockname failed");
  }
  char buffer[1024] = "Hello brutha";
  if (send(clientsocket, buffer, sizeof(buffer), 0) < 0) {
    cout << "Failed to send the data" << endl;
    close(clientsocket);
    return EXIT_FAILURE;
  }
}
