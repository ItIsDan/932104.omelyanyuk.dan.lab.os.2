#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define MAX_CLIENTS 5

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int sigNumber) {
        wasSigHup = 1;
}

int main() {
    int serverSocket, clientSocket, maxFd, activeClients = 0;
    fd_set fds;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int i;

    // Создание сокета
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Инициализация структуры адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080); 

    // Привязка сокета к адресу и порту
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error binding socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Подготовка к прослушиванию соединений
    if (listen(serverSocket, 5) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d \n", 8080);

    // Регистрация обработчика сигнала
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    // Блокировка сигнала
    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigemptyset(&origMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    // Основной цикл приложения
    while (activeClients < 5) {

        // Инициализация множества файловых дескрипторов
        FD_ZERO(&fds);
        FD_SET(serverSocket, &fds);
        maxFd = serverSocket;

        fd_set tmpFds = fds; 

        if (pselect(maxFd + 1, &tmpFds, NULL, NULL, NULL, &origMask) < 0) {
            if (errno != EINTR) { 
                perror("Error in pselect");
                exit(EXIT_FAILURE); 
            }
        }
        if (wasSigHup) {
                printf("Received SIGHUP signal. Exiting.\n");
                wasSigHup = 0;
            	activeClients++;
	            continue;
                }

        // Проверка активности на серверном сокете
        if (FD_ISSET(serverSocket, &tmpFds)) {
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                perror("Error accepting connection");
                exit(EXIT_FAILURE);
            }
            char buffer[1024] = { 0 };
	    int readBuffer = read(clientSocket, buffer, sizeof(buffer));
	    if (readBuffer > 0) 
                printf("Received text [ %s ] from user %d\n", buffer, activeClients + 1); 
       	    else {
       	        if (readBuffer == 0) 
                    close(serverSocket); 
                else  
            	    perror("Read Error");  
            } 
            activeClients++;       
      }

      // Добавление нового клиента в множество файловых дескрипторов
      FD_SET(clientSocket, &fds);

      // Обновление максимального значения дескриптора
      if (clientSocket > maxFd) 
          maxFd = clientSocket;          
    
    char* fromServer = "Received!";
    send(clientSocket, fromServer, strlen(fromServer), 0);
    
    }

    close(serverSocket);
    
    return 0;
}
