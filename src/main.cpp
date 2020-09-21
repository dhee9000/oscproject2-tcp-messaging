/**
 * 
 * Author: Dheeraj Yalamanchili
 * Email: dhee9000@gmail.com
 * 
 * main.cpp
 * 
 */

#include <iostream>
#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <time.h>
#include <pthread.h>

// Configuration Parameters
const char *MACHINE_IPS[] = {"127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1"};
#define PORT 7777
#define MAX_CONNECTIONS 4
#define BUFFER_SIZE 2048

using namespace std;

bool stopped = false;

void *server(void *data)
{
    int socket_fd = *((int *)data);
    char buffer[BUFFER_SIZE];
    while (1)
    {
        if (recv(socket_fd, buffer, BUFFER_SIZE, 0) < 0)
        {
            printf("Error receiving message!\n");
            exit(EXIT_FAILURE);
        }
        printf("%s\n", buffer);
        if(strcmp(buffer, "stop") == 0){
            stopped = true;
            cout << "stopped by remote command\n";
            close(socket_fd);
            pthread_exit(0);
        }
    }
    return 0;
}

void *spawnServer(void *data)
{
    int machine_id = *((int *)data);
    int socket_desc, socket_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_storage srv_storage;
    const char *ip = MACHINE_IPS[machine_id];

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Error creating server socket!\n");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(PORT);

    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Error binding socket to port!\n");
        exit(EXIT_FAILURE);
    }

    cout << "Creating Server..." << endl;

    if (listen(socket_desc, MAX_CONNECTIONS) < 0)
    {
        printf("Error listening for connections!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t thread_ids[MAX_CONNECTIONS];
    int active_connections = 0;

    while (1)
    {
        socklen_t addr_size = sizeof(srv_storage);
        if ((socket_fd = accept(socket_desc, (struct sockaddr *)&srv_storage, &addr_size)) < 0)
        {
            printf("Error accepting socket connection!\n");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&thread_ids[active_connections++], NULL, server, &socket_fd) != 0)
        {
            printf("Error creating thread for connection!\n");
            exit(EXIT_FAILURE);
        }

        if(stopped){
            pthread_exit(0);
        }

        //Wait for server threads to end
        if (active_connections >= MAX_CONNECTIONS)
        {
            for (int i = 0; i < MAX_CONNECTIONS; i++)
                pthread_join(thread_ids[i], NULL);
            pthread_exit(0);
        }
    }
    return 0;
}

void *spawnClient(void *data)
{
    int machine_id = *((int *)data);
    int socket_fd[4];
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {

        cout << "Creating Client " << i << "..." << endl;

        socket_fd[i] = socket(AF_INET, SOCK_STREAM, 0);

        server_addr.sin_addr.s_addr = inet_addr(MACHINE_IPS[i]);

        if (socket_fd[i] == -1)
        {
            printf("Error creating client socket!\n");
            exit(EXIT_FAILURE);
        }

        if (connect(socket_fd[i], (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            printf("Error binding socket to port!\n");
            exit(EXIT_FAILURE);
        }
    }

    cout << "Ready for Commands!" << endl;

    char buffer[1024];
    while (!stopped)
    {
        string command, message;
        int target;
        cin >> command;
        if (command == "stop")
        {
            stopped = true;
            strcpy(buffer, "stop");
            for (int i = 0; i < 4; i++){
                send(socket_fd[i], buffer, strlen(buffer), 0);
                close(socket_fd[i]);
            }
            cout << "stopped by input" << endl;
            pthread_exit(0);
        }
        if (command == "send")
        {
            cin >> target;
            cin >> message;
            if(target == machine_id)
                cout << "(sending to self)" << endl;
            strcpy(buffer, message.c_str());
            send(socket_fd[target], buffer, strlen(buffer), 0);
            cout << "message sent!" << endl;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Please specify the machine id as an argument. e.g. main 0\n");
        exit(EXIT_FAILURE);
    }

    char *p;
    int machine_id = strtol(argv[1], &p, 10);

    pthread_t serverThread, clientThread;

    cout << "Starting All Threads..." << endl;

    if (pthread_create(&serverThread, NULL, spawnServer, &machine_id) != 0)
    {
        printf("Error creating spawn thread for server!\n");
        exit(EXIT_FAILURE);
    }
    // Delay starting of client threads to allow other servers to launch
    sleep(10);
    if (pthread_create(&clientThread, NULL, spawnClient, &machine_id) != 0)
    {
        printf("Error creating spawn thread for server!\n");
        exit(EXIT_FAILURE);
    }

    // End as soon as stopped is true
    while(!stopped){
        sleep(1);
    }

    time_t ltime;
    ltime=time(NULL);
    printf("Program Clean Exit at %s\n",asctime( localtime(&ltime) ) );
}