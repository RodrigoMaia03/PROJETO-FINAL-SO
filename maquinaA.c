#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define MAX_CLIENTS 2

int main() {
    int serverSocket, clientSockets[MAX_CLIENTS];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t addressLength = sizeof(clientAddress);
    char buffer[1024];

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erro ao criar o socket do servidor");
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Erro ao vincular o socket à porta");
        close(serverSocket);
        exit(1);
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        perror("Erro ao escutar por conexões");
        close(serverSocket);
        exit(1);
    }

    printf("Servidor iniciado e aguardando por conexões...\n");

    int numClients = 0;
    while (numClients < MAX_CLIENTS) {
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addressLength);
        if (clientSocket == -1) {
            perror("Erro ao aceitar a conexão do cliente");
            close(serverSocket);
            exit(1);
        }

        clientSockets[numClients] = clientSocket;
        numClients++;

        printf("Novo cliente conectado. Total de clientes: %d\n", numClients);
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        // Receber uma mensagem de qualquer cliente
        int bytesRead = recv(clientSockets[0], buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Erro ao receber a mensagem do cliente");
            break;
        }

        // Enviar a mensagem para o outro cliente
        send(clientSockets[1], buffer, bytesRead, 0);

        // Receber uma mensagem do outro cliente
        bytesRead = recv(clientSockets[1], buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Erro ao receber a mensagem do cliente");
            break;
        }

        // Enviar a mensagem para o primeiro cliente
        send(clientSockets[0], buffer, bytesRead, 0);
    }

    for (int i = 0; i < numClients; i++) {
        close(clientSockets[i]);
    }

    close(serverSocket);

    return 0;
}
