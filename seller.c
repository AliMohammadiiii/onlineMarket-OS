#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define TCPPORT 8082
#define BROADCASTPORT 5000

int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

int broadcastSetup(int port)
{

}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, max_sd, broadcast_fd;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
       
    server_fd = setupServer(TCPPORT);
    
    
    int broadcastPort = atoi(argv[1]);

    int broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;


    broadcast_fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(broadcast_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(broadcast_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(broadcastPort); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");

    bind(broadcast_fd, (struct sockaddr *)&bc_address, sizeof(bc_address));




    FD_ZERO(&master_set);
    if(server_fd > broadcast_fd)
        max_sd = server_fd;
    else
        max_sd = broadcast_fd;

    FD_SET(server_fd, &master_set);
    FD_SET(broadcast_fd, &master_set);
    FD_SET(0, &master_set);

    write(1, "Server is running\n", 18);

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                printf("fd id: %d\n", i);
                if (i == server_fd) {  // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                }
                if (i == 0)
                {
                    memset(buffer, 0, 1024);
                    read(0, buffer, 1024);
                    int a = sendto(broadcast_fd, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                }
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, 1024, 0);
                    
                    if (bytes_received == 0) { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    printf("client %d: %s\n", i, buffer);
                    memset(buffer, 0, 1024);
                }
            }
        }

    }

    return 0;
}
