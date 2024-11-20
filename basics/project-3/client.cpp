#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread> // For multithreading
#include <unistd.h>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1111

void receive_messages(int sockfd) {
  char buffer[1024];
  struct sockaddr_in serveraddr;
  socklen_t addrlen = sizeof(serveraddr);

  while (true) {
    int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                  (struct sockaddr *)&serveraddr, &addrlen);
    if (bytes_received < 0) {
      perror("Error receiving message");
      break;
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received message
    cout << "Broadcast from server: " << buffer << endl;
  }
}

int main() {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("Failed to create socket");
    return EXIT_FAILURE;
  }

  struct sockaddr_in serveraddr;
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_IP, &serveraddr.sin_addr);

  cout << "Connected to UDP server at " << SERVER_IP << ":" << SERVER_PORT
       << endl;

  // Start a thread for receiving messages
  thread receiver(receive_messages, sockfd);

  // Sending messages
  char message[1024];
  while (true) {
    cout << "Enter message to send: ";
    cin.getline(message, sizeof(message));

    int bytes_sent = sendto(sockfd, message, strlen(message), 0,
                            (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (bytes_sent < 0) {
      perror("Failed to send message");
      break;
    }
  }

  // Cleanup
  receiver.detach(); // Allow the thread to exit gracefully
  close(sockfd);
  return 0;
}
