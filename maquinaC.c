#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345

int clientSocket;

void *inputThread(void *arg) {
    char message[1024];

    while (1) {
        fgets(message, sizeof(message), stdin);

        if (strncmp(message, "sair", 4) == 0) {
            break;
        }

        if (send(clientSocket, message, strlen(message), 0) < 0) {
            perror("Erro ao enviar a mensagem para o servidor");
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

void *receiveThread(void *arg) {
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        // Receber uma mensagem do servidor
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Erro ao receber a mensagem do servidor");
            break;
        }

        printf("%s", buffer);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t inputThreadId, receiveThreadId;
    struct sockaddr_in serverAddress;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar o socket do cliente");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &(serverAddress.sin_addr)) <= 0) {
        perror("Endereço de servidor inválido");
        exit(EXIT_FAILURE);
    }

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Erro ao conectar ao servidor");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&inputThreadId, NULL, inputThread, NULL) != 0) {
        perror("Erro ao criar a thread de entrada");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&receiveThreadId, NULL, receiveThread, NULL) != 0) {
        perror("Erro ao criar a thread de recebimento");
        exit(EXIT_FAILURE);
    }

    pthread_join(inputThreadId, NULL);
    pthread_join(receiveThreadId, NULL);

    close(clientSocket);

    return 0;
}