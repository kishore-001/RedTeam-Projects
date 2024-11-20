#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;

#define PORT 1111

vector<struct sockaddr_in> clientlist;

// Check if client is already in the list
bool ifclientinlist(struct sockaddr_in &client) {
  for (const auto &addr : clientlist) {
    if (addr.sin_port == client.sin_port &&
        addr.sin_addr.s_addr == client.sin_addr.s_addr) {
      return true;
    }
  }
  return false;
}

// Add new client to the list
void add_client_struct(struct sockaddr_in &client) {
  if (!ifclientinlist(client)) {
    clientlist.push_back(client);
    cout << "New Client added to the list | " << inet_ntoa(client.sin_addr)
         << ":" << ntohs(client.sin_port) << endl;
  }
}

// Broadcast the message to all clients except the sender
void broadcast(int srvfd, const char *message,
               const struct sockaddr_in &sender) {
  for (const auto &addr : clientlist) {
    if (sender.sin_addr.s_addr == addr.sin_addr.s_addr &&
        sender.sin_port == addr.sin_port) {
      continue; // Skip sender
    }

    int sendval = sendto(srvfd, message, strlen(message), 0,
                         (struct sockaddr *)&addr, sizeof(addr));
    if (sendval < 0) {
      cout << "Failed to send the data to the client: "
           << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;
    }
  }
}

int main() {
  int srvfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (srvfd < 0) {
    perror("Failed to create a server socket");
    return EXIT_FAILURE;
  }
  cout << "Successfully created a server socket..." << endl;

  // Set socket to non-blocking mode
  int flags = fcntl(srvfd, F_GETFL, 0);
  if (flags < 0) {
    perror("Failed to get socket flags");
    close(srvfd);
    return EXIT_FAILURE;
  }

  if (fcntl(srvfd, F_SETFL, flags | O_NONBLOCK) < 0) {
    perror("Failed to set socket to non-blocking mode");
    close(srvfd);
    return EXIT_FAILURE;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (bind(srvfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Failed to bind the socket");
    close(srvfd);
    return EXIT_FAILURE;
  }

  cout << "Server started to listen on port " << PORT << endl;

  char buffer[1024];
  struct sockaddr_in clientinfo;
  socklen_t clientval = sizeof(clientinfo);

  while (true) {
    cout << "Waiting for client" << endl;
    int data = recvfrom(srvfd, buffer, sizeof(buffer) - 1, 0,
                        (struct sockaddr *)&clientinfo, &clientval);

    if (data < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available, continue the loop
        usleep(1000000); // Optional: Small sleep to avoid busy-waiting
        continue;
      } else {
        perror("Error receiving data");
        break;
      }
    }

    buffer[data] = '\0'; // Null-terminate received data
    char datafromclient[2048];

    snprintf(datafromclient, sizeof(datafromclient),
             "Message from IP: %s PORT: %d\n%s", inet_ntoa(clientinfo.sin_addr),
             ntohs(clientinfo.sin_port), buffer);

    cout << datafromclient << endl;

    add_client_struct(clientinfo);
    broadcast(srvfd, datafromclient, clientinfo);
  }

  close(srvfd);
  return 0;
}
