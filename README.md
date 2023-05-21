# ИДЗ 3, Зиганшин Алим, БПИ216, Вариант 4

Выполнены пункты на оценку 4-5 и 6-7. Для программ на 4, 5 и 6 один вывод (Все потоки внутри одной программы). Код программы и результаты работы предоставлены в отчете. Для завершения работы программы можно использовать Ctrl + C.

## Задача:

> Задача о читателях и писателях. Базу данных разделяют два
типа процессов – читатели и писатели. Читатели выполняют транзакции, которые просматривают записи базы данных, транзакции
писателей и просматривают и изменяют записи. Предполагается,
что в начале БД находится в непротиворечивом состоянии (например, если каждый элемент — число, то они все отсортированы).
Каждая отдельная транзакция переводит БД из одного непротиворечивого состояния в другое. Для предотвращения взаимного влияния транзакций процесс-писатель должен иметь исключительный
доступ к БД. Если к БД не обращается ни один из процессовписателей, то выполнять транзакции могут одновременно сколько
угодно читателей. Создать приложение с процессами–писателями и процессами–читателями.
> 

## Сценарий решаемой задачи

Поскольку количество писателей не оговорено, я решил ввести одного писателя и 3 читателей (такого количество достаточно, для проверки их взаимодейсвтия)

В данном случае, писатель пишет сообщения не прирываясь ни на секунду. Читатели же, каждые две секунды захватывают мьютекс и пытаются прочитать.

(в случае с наблюдателем, он выводит все, что видит, т.е. в основном запись)

Формат вывода таков: Писатель выводит в консоль, что он записал, а читатель - что он прочитал

## Код читателя, писателя, наблюдателя
Этот код не менялся, поэтому предлагаю ознакомиться сразу:

Писатель: генерирует случайное число до 100, которое передает в запросе на сервер. Сервер понимает, что если пришло число, то это писатель, и возвращает это-же число, чтоб подтвердить успешность записи.
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

void start_client(const char* server_ip, int server_port) {
    int sock = 0, valread;
    srand(time(NULL));
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server\n");
    while(1) {
        // Send a transaction request to the server
        int num = rand() % 100;
        char message[128];
        snprintf(message, sizeof(message), "%d\n", num);
        send(sock, message, strlen(message), 0);
       // Receive the current database value from the server
       memset(buffer, 0, BUFFER_SIZE);
       valread = recv(sock, buffer, BUFFER_SIZE, 0);
       if (valread > 0) {
           printf("Writer written: %s\n", buffer);
       } else {
           printf("Write failed\n");
       }
    }

    // Close the socket
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    start_client(server_ip, server_port);

    return 0;
}
```

Читатель: отправляет запрос с текстом "read", а в ответ получает текущее состояние базы данных (т.е. число), после чего ждет 2 секунды, чтобы снова отправить запрос.
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int name;

void start_client(const char* server_ip, int server_port) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server\n");
    while(1) {
        // Send a transaction request to the server
        send(sock, "read", strlen("read"), 0);
       // Receive the current database value from the server
       memset(buffer, 0, BUFFER_SIZE);
       valread = recv(sock, buffer, BUFFER_SIZE, 0);
       if (valread > 0) {
           printf("Reader number %d read: %s\n", name, buffer);
       } else {
           printf("Read failed\n");
       }
    }

    // Close the socket
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Port num and name required");
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    name = atoi(argv[3]);

    start_client(server_ip, server_port);

    return 0;
}

```

Наблюдатель: постоянно получает от сервера информацию о каком либо событии (запись или чтение) и выводит ее.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024


void start_client(const char* server_ip, int server_port) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server\n");
    while(1) {
       memset(buffer, 0, BUFFER_SIZE);
       valread = recv(sock, buffer, BUFFER_SIZE, 0);
       if (valread > 0) {
           printf("Viewer noticed that: %s\n", buffer);
       } else {
           printf("Viewing failed\n");
       }
    }

    // Close the socket
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Port num required");
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    start_client(server_ip, server_port);

    return 0;
}
```
## Вывод программ на оценку 4-5

![4-5point.png](https://github.com/hggfun/OS_IDZ3/blob/main/4-5point.png)

## Отчет на 4-5 балла
И так, сервер в мейне получает адрес хоста и порта, разворачивается на нем, в методе старт_сервер. В нем он инициализирует также читателей и писателей, каждому из которых вызывается метод, в котором клиент может захватить мьютекс, прочитать/записать и отпустить мьютекс. И так по кругу.
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_writer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_reader = PTHREAD_COND_INITIALIZER;

int writer_count = 0;
int reader_count = 0;
int database = 0;

void* client_thread(void* arg) {
    int client_socket = *((int*)arg);
    int wait = 2;
    char buffer[BUFFER_SIZE];
    while(1) {
// Lock the mutex
    pthread_mutex_lock(&mutex);
    // Receive data from the client
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (buffer[0] != 'r') {
        database = atoi(buffer);
        wait = 0;
    }
    // Send the current database value to the client
    sprintf(buffer, "%d", database);
    send(client_socket, buffer, strlen(buffer), 0);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex);
    sleep(wait);
    }
    
    // Close the client socket
    close(client_socket);

    pthread_exit(NULL);
}

void start_server(const char* host, int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d\n", host, port);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted writer\n");
        }

        // Create a thread for the client
        pthread_t thread;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread, NULL, client_thread, client_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread
        pthread_detach(thread);

    // Accept incoming connections and create threads for handling clients
    for (int i = 0; i < 3; ++i) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted reader number %d\n", i + 1);
        }

        // Create a thread for the client
        pthread_t thread;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread, NULL, client_thread, client_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread
        pthread_detach(thread);
    }
    while(1) {
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);

    start_server(host, port);

    return 0;
}
```
## Вывод программы на 6-7 баллов
![6-7point.png](https://github.com/hggfun/OS_IDZ3/blob/main/6-7point.png)
## Отчет на 6-7 баллов

Почти все аналогично предыдущему пункту, только теперь инициализируется еще и наблюдатель. Метод для обработки теперь отправляет информацию не только конкретному клиенту, но и наблюдателю.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_writer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_reader = PTHREAD_COND_INITIALIZER;

int writer_count = 0;
int reader_count = 0;
int database = 0;
int* viewer;

void* client_thread(void* arg) {
    int client_socket = *((int*)arg);
    int viewer_socket = *((int*)viewer);
    int wait = 2;
    char buffer[BUFFER_SIZE];
    while(1) {
// Lock the mutex
    pthread_mutex_lock(&mutex);
    // Receive data from the client
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (buffer[0] != 'r') {
        database = atoi(buffer);
        wait = 0;
        send(viewer_socket, "writer is writing", strlen("writer is writing"), 0);
    } else {
        send(viewer_socket, "reader is reading", strlen("reader is reading"), 0);
    }
    // Send the current database value to the client
    sprintf(buffer, "%d", database);
    send(client_socket, buffer, strlen(buffer), 0);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex);
    sleep(wait);
    }
    
    // Close the client socket
    close(client_socket);

    pthread_exit(NULL);
}

void start_server(const char* host, int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d\n", host, port);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted viewer\n");
        }
        viewer = malloc(sizeof(int));
        *viewer = new_socket;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted writer\n");
        }

        // Create a thread for the client
        pthread_t thread;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread, NULL, client_thread, client_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread
        pthread_detach(thread);

    // Accept incoming connections and create threads for handling clients
    for (int i = 0; i < 3; ++i) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted reader number %d\n", i + 1);
        }

        // Create a thread for the client
        pthread_t thread;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread, NULL, client_thread, client_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread
        pthread_detach(thread);
    }
    while(1) {
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);

    start_server(host, port);

    return 0;
}

```
