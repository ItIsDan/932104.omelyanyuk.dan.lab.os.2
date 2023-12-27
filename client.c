#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main() {
    struct sockaddr_in serverAddr;

    // Создание сокета
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Инициализация структуры адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        printf("Error: Address not supported \n");
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Error connecting to server\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    } 
    printf("Successful connection\n");
    
    // Запрос текста
    char msg[1024] = { 0 };
    printf("Enter you text: ");
    fgets(msg, 1024, stdin);
    
    const char* message = msg;

    // Отправка данных на сервер
    int result = send(clientSocket, message, strlen(message) - 1, 0); 
    
    if (result == -1) {
        perror("\nError sending data\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    } else {	
    	printf("Message sent\n");
	}
    
    // Закрытие соединения
    close(clientSocket);
    
    return 0;
}
