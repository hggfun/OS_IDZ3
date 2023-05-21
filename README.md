# ИДЗ 2, Зиганшин Алим, БПИ216, Вариант 4

Выполнены пункты на оценку 4, 5, 6, 7. Для программ на 4, 5 и 6 один вывод (Все потоки внутри одной программы). На 7 другой формат вывода (т. к. несколько единиц компиляции). Код программы и результаты работы предоставлены в отчете. Для завершения работы программы можно использовать Ctrl + C.

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

Поскольку количество писателей не оговорено, я решил ввести 2 писателей и 4 читателей (такого количество достаточно, для проверки их взаимодейсвтия)

Я использую POSIX для хранения семафоров. Изначально писатель пишет в файл. Когда дописывает, либо читатели успевают начать читать, либо какой-либо писатель успевает начать снова писать (и читатели ждут пока он допишет снова)

Каждый семафор и изначальная запись инициализируются в начале работы программы, в конце работы память освобождается.

Формат вывода таков: Писатель выводит в консоль, что он записал, а читатель - что он прочитал

## Вывод программ на оценку 4, 5 и 6

по сути меняли только типы работы с памятью и семафорами. Вывод везде одинаковый (при разных запусках может различаться порядок вывода, в силу рандомности)

![4-6point.png](https://github.com/hggfun/OS_IDZ2/blob/main/4-6point.png)

## Отчет на 4 балла

Процессы писателей и читателей взаимодействуют через именованные POSIX семафоры. Разделяемая память POSIX используется для хранения базы данных (БД - условно, вместо нее я использую обычный int)

```c
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/mman.h>

#define MAX_READERS 4 // Максимальное количество читателей
#define MAX_WRITERS 2  // Максимальное количество писателей

int database = 0;  // Изначальное состояние базы данных
int readers = 0;   // Количество читателей, которые сейчас читают базу данных

sem_t mutex;       // Семафор для исключительного доступа к базе данных
sem_t db_access;   // Семафор для синхронизации доступа читателей и писателей

void* reader(void* arg) {
    int id = *(int*) arg;
    while (1) {
        // Захват доступа к базе данных
        sem_wait(&db_access);

        // Чтение базы данных
        printf("Reader %d is reading from database %d\n", id, database);
        sleep(1);
        sem_post(&db_access);

        // Перерыв между чтениями
        sleep(1);
    }
    return NULL;
}

void *writer(void *arg) {
    int id = *(int*)arg;
    while (1) {
        sem_wait(&db_access); // Захват db_access, чтобы другие писатели и читатели не могли получить доступ к базе данных
        // Изменение базы данных
        database += 1;
        printf("Writer %d is writing to the database: %d\n", id, database);
        sleep(1);
        sem_post(&db_access); // Освобождение db_access
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t readers_threads[MAX_READERS];
    pthread_t writers_threads[MAX_WRITERS];

    sem_init(&mutex, 0, 1);
    sem_init(&db_access, 0, 1);

    // Создание читателей
    for (int i = 0; i < MAX_READERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&readers_threads[i], NULL, reader, id);
    }

    // Создание писателей
    for (int i = 0; i < MAX_WRITERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&writers_threads[i], NULL, writer, id);
        }
    // Ожидание завершения потоков
    for (int i = 0; i < MAX_READERS; i++) {
        pthread_join(readers_threads[i], NULL);
        }

    for (int i = 0; i < MAX_WRITERS; i++) {
        pthread_join(writers_threads[i], NULL);
        }

    // Освобождение ресурсов
    sem_destroy(&mutex);
    sem_destroy(&db_access);
    return 0;
}
```

## Отчет на 5 баллов

Почти все аналогично предыдущему пункту, только теперь семафоры POSIX неименованные. Хранение и редактирование БД также через разделяемую память стандарта POSIX.

```c
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/mman.h>

#define MAX_READERS 4 // Максимальное количество читателей
#define MAX_WRITERS 2  // Максимальное количество писателей

int database = 0;  // Изначальное состояние базы данных
int readers = 0;   // Количество читателей, которые сейчас читают базу данных

sem_t *mutex;       // Семафор для исключительного доступа к базе данных
sem_t *db_access;   // Семафор для синхронизации доступа читателей и писателей

void* reader(void* arg) {
    int id = *(int*) arg;
    while (1) {
        // Захват доступа к базе данных
        sem_wait(db_access);

        // Чтение базы данных
        printf("Reader %d is reading from database %d\n", id, database);
        sleep(1);
        sem_post(db_access);

        // Перерыв между чтениями
        sleep(1);
    }
    return NULL;
}

void *writer(void *arg) {
    int id = *(int*)arg;
    while (1) {
        sem_wait(db_access); // Захват db_access, чтобы другие писатели и читатели не могли получить доступ к базе данных
        // Изменение базы данных
        database += 1;
        printf("Writer %d is writing to the database: %d\n", id, database);
        sleep(1);
        sem_post(db_access); // Освобождение db_access
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t readers_threads[MAX_READERS];
    pthread_t writers_threads[MAX_WRITERS];
    mutex = sem_open("mutex", O_CREAT, 0644, 0);
    db_access = sem_open("db_access", O_CREAT, 0644, 0);

    // Создание читателей
    for (int i = 0; i < MAX_READERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&readers_threads[i], NULL, reader, id);
    }

    // Создание писателей
    for (int i = 0; i < MAX_WRITERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&writers_threads[i], NULL, writer, id);
        }
    // Ожидание завершения потоков
    for (int i = 0; i < MAX_READERS; i++) {
        pthread_join(readers_threads[i], NULL);
        }

    for (int i = 0; i < MAX_WRITERS; i++) {
        pthread_join(writers_threads[i], NULL);
        }

    // Освобождение ресурсов
    sem_close(mutex);
    sem_unlink("mutex");
    sem_close(db_access);
    sem_unlink("db_access");
    return 0;
}
```

## Отчет на 6 баллов

Теперь за взаимодействие писателя и читателей отвечают семафоры UNIX SYSTEM V. БД хранится и редактируется через разделяемую память в стандарте UNIX SYSTEM V.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_WRITERS 2
#define MAX_READERS 3

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

struct shared_mem {
    int database;
};

int sem_id, shm_id;
void* shm_ptr;
struct shared_mem* shared_memory;

void reader(int id, int database) {
    while (1) {
        // Ждем, пока писатель закончит запись
        struct sembuf wait_writer = {0, -1, SEM_UNDO};
        semop(sem_id, &wait_writer, 1);

        // Проверяем доступ к бд и читаем
        printf("Reader %d is reading from database %d\n", id, database);
            sleep(1);

            // Завершаем работу с бд
            struct sembuf finish_reading = {0, 1, SEM_UNDO};
            semop(sem_id, &finish_reading, 1);
    }
}

void writer() {
    while (1) {
        // Изменим запись в бд
        shared_memory->database++;

        printf("Writer 0 is writing to the database: %d\n", shared_memory->database);

        // Разрешаем чтение читателям
        struct sembuf allow_readers = {0, MAX_READERS, SEM_UNDO};
        semop(sem_id, &allow_readers, 1);

        // Ждем, пока читатели дочитывают
        struct sembuf wait_readers = {0, 0, SEM_UNDO};
        semop(sem_id, &wait_readers, 1);
        sleep(1);
    }
}

int main() {
    // Создаем и инициализируем cемафор
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    union semun sem_union;
    sem_union.val = 0;
    semctl(sem_id, 0, SETVAL, sem_union);

    // Создаем и инициализируем разделяемую память
    shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_mem), IPC_CREAT | 0666);
    shm_ptr = shmat(shm_id, NULL, 0);
    shared_memory = (struct shared_mem*) shm_ptr;

    // Инициализируем генератор случайных чисел
    srand(time(NULL));

    // Создаем читателей
    for (int i = 0; i < MAX_READERS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("Читатель не создан\n");
            return 1;
        } else if (pid == 0) {
            reader(i + 1, i + 1);
            return 0;
        }
    }

    // Создаем писателей
    pid_t pid = fork();
    if (pid < 0) {
        printf("Писатель не создан\n");
        return 1;
    } else if (pid == 0) {
        writer();
        return 0;
    }

    // Ждем завершения дочерних процессов
    for (int i = 0; i < MAX_READERS + 1; i++) {
        wait(NULL);
    }

    // Освобождаем ресурсы
    semctl(sem_id, 0, IPC_RMID, sem_union);
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
```

## Вывод программы на оценку 7

![7point.png](https://github.com/hggfun/OS_IDZ2/blob/main/7point.png)
## Отчет на 7 баллов

Запускаю процессы читателей и писателей (соответственно 4 чтателя и 2 писателя). 

Множество независимых процессов взаимодействуют с использованием именованных POSIX семафоров. Обмен данными ведется через разделяемую память в стандарте POSIX

**Читатель:**

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>

sem_t *reader_semaphores[4];
sem_t *mutex;
int *database;
int id;

int main(int argc, char *argv[]) {
    id = argv[1];
    mutex = sem_open("mutex", O_CREAT, 0644, 1);
    reader_semaphores[0] = sem_open("reader1", O_CREAT, 0644, 0);
    reader_semaphores[1] = sem_open("reader2", O_CREAT, 0644, 0);
    reader_semaphores[2] = sem_open("reader3", O_CREAT, 0644, 0);
    reader_semaphores[3] = sem_open("reader4", O_CREAT, 0644, 0);
    int shm = shm_open("shm", O_RDWR, 0644);
    ftruncate(shm, 2 * sizeof(int));
    database = mmap(NULL, 2 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    while (1) {
        sem_wait(reader_semaphores[id]);
        sleep(2);
        printf("Reader %d is reading from database %d\n", id, database[0]);
    }
    sem_close(mutex);
    sem_close(reader_semaphores[0]);
    sem_close(reader_semaphores[1]);
    sem_close(reader_semaphores[2]);
    sem_close(reader_semaphores[3]);
    sem_unlink("reader1");
    sem_unlink("reader2");
    sem_unlink("reader3");
    sem_unlink("reader4");
    sem_unlink("mutex");
    return 0;
}
```

**Писатель:**

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

sem_t *reader_semaphores[4];
sem_t *mutex;
int *database;

int main() {
    srand(time(NULL));
    mutex = sem_open("mutex", O_CREAT, 0644, 1);
    reader_semaphores[0] = sem_open("reader1", O_CREAT, 0644, 0);
    reader_semaphores[1] = sem_open("reader2", O_CREAT, 0644, 0);
    reader_semaphores[2] = sem_open("reader3", O_CREAT, 0644, 0);
    reader_semaphores[3] = sem_open("reader4", O_CREAT, 0644, 0);
    int shm = shm_open("shm", O_RDWR, 0644);
    ftruncate(shm, 2 * sizeof(int));
    database = mmap(NULL, 2 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    database[0] = -1;
    while (1) {
        database[0] = database[0] + 1;
        int k = rand() % 2;
        printf("Writer %d is writing to the database: %d\n", k, database[0]);
        k = rand() % 5;
        for (int i = k; i > 0; i--) {
        int t = rand() % 4;
        sem_post(reader_semaphores[t]);
        sleep(3);
        sem_wait(mutex);  
        sleep(1);
        }
    }
    sem_close(mutex);
    sem_close(reader_semaphores[0]);
    sem_close(reader_semaphores[1]);
    sem_close(reader_semaphores[2]);
    sem_close(reader_semaphores[3]);
    sem_unlink("reader1");
    sem_unlink("reader2");
    sem_unlink("reader3");
    sem_unlink("reader4");
    sem_unlink("mutex");
    return 0;
}
```
