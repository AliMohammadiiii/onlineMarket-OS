#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define TCPPORT 8081
#define BROADCASTPORT 5000


int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    printf("connecting to %d\n", port);
    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }
    printf("connected %d\n", port);

    return fd;
}


int main(int argc, char const *argv[]) {
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;
    int broadcastSock,etcpSock, listonBroadcastPort, listonTCPPort;
    int server_fd, new_socket, max_sd;
    fd_set master_set, working_set;



    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(BROADCASTPORT); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");
    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));


    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(sock, &master_set);
    FD_SET(0, &master_set);
    int flag = 0;
    while (1) {
        flag = 0;
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);


        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == 0) {  // new clinet
                    read(0,buffer,sizeof(buffer));
                    printf("type: %s", buffer);
                    int fd = connectServer(TCPPORT);
                    read(0, buffer, 1024);
                    send(fd, buffer, strlen(buffer), 0);
                    memset(buffer, 0, 1024);
                    close(fd);
                    flag = 1;
                }
                
            }
        }
        if (flag == 0)
        {
            memset(buffer, 0, 1024);
            recv(sock, buffer, 1024, 0);
            printf("%s\n", buffer);
        }
    }

    return 0;
}