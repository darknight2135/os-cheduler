#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
const int N = 1e5;

typedef struct Process
{
    int pid;
    int real;
    int state;
    int priority;
    time_t start_time;
    time_t completion_time;
    char name[50];
} Process;

typedef struct Queue
{
    int front;
    int rear;
    int capacity;
    Process data[10000];
} Queue;

Queue *createQueue(int capacity)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue) + capacity * sizeof(Process));
    queue->front = 0;
    queue->rear = -1;
    queue->capacity = capacity;
    return queue;
}
Queue *currqueue;
bool isQueueEmpty(Queue *queue)
{
    return queue->front > queue->rear;
}

bool isQueueFull(Queue *queue)
{
    return queue->rear == queue->capacity - 1;
}

void enqueue(Queue *queue, Process element)
{
    if (isQueueFull(queue))
    {
        return;
    }

    int index = queue->rear;
    while (index >= queue->front && element.priority < queue->data[index].priority)
    {
        queue->data[index + 1] = queue->data[index];
        index--;
    }

    queue->data[index + 1] = element;
    queue->rear++;
}
void enqueue2(Queue *queue, Process element)
{
    if (isQueueFull(queue))
    {
        return;
    }
    queue->data[++queue->rear] = element;
}

Process dequeue(Queue *queue)
{
    if (isQueueEmpty(queue))
    {
        Process nullProcess = {-1, -1};
        return nullProcess;
    }

    Process dequeuedElement = queue->data[queue->front];
    queue->front++;

    return dequeuedElement;
}

char *commandHistory[1024];
typedef struct
{
    Queue queue;
    int NPROCS;
    int TSLICE;
    char history[1024][1024];
    int historyCount;
} SharedMemory;
SharedMemory *shared_mem;

void addhistory(char *command, double timeTaken, pid_t cpid)
{
    time_t currentTime = time(NULL);
    pid_t pid = cpid;
    char *commandCopy = strdup(command);
    commandHistory[shared_mem->historyCount] = commandCopy;
    char temp[1000];
    struct tm *timeInfo = localtime(&currentTime);

    sprintf(temp, "[%02d:%02d:%02d] PID: %d - submit %s (Time taken: %.6f seconds)",
            timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec,
            pid, command, timeTaken);

    strcpy(shared_mem->history[shared_mem->historyCount], temp);
    shared_mem->historyCount++;
}
void printHistory()
{
    printf("History:\n");
    for (int i = 0; i < shared_mem->historyCount; i++)
    {
        printf("%d: %s\n", i + 1, shared_mem->history[i]);
    }
}
int main()
{
    printf("Scheduler started\n");
    pid_t pid;
    int shm_fd = shm_open("my_shared_memory", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1)
    {
        perror("Error sizing shared memory");
        exit(EXIT_FAILURE);
    }

    shared_mem = (SharedMemory *)mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED)
    {
        perror("Error mapping shared memory");
        exit(EXIT_FAILURE);
    }
    int sem_id = semget(IPC_PRIVATE, 1, 0666);
    if (sem_id == -1)
    {
        perror("Error creating semaphore");
        exit(EXIT_FAILURE);
    }
    int k = 0;
    while (1)
    {
        fflush(stdout);
        while (isQueueEmpty(&shared_mem->queue))
        {
            continue;
        }
        currqueue = createQueue(10000);
        int i = 0;
        while (i < shared_mem->NPROCS)
        {
            if (!isQueueEmpty(&shared_mem->queue))
            {
                Process curr = dequeue(&shared_mem->queue);
                if (curr.real == 1)
                {
                    enqueue2(currqueue, curr);
                }
            }
            else
            {
                break;
            }
            i++;
        }
        while (!isQueueEmpty(currqueue))
        {

            for (int i = 0; i < shared_mem->NPROCS; i++)
            {
                bool test = isQueueEmpty(currqueue);

                fflush(stdout);
                if (isQueueEmpty(currqueue))
                {
                    continue;
                }
                Process current_process = dequeue(currqueue);
                if (current_process.real == 0)
                {
                    continue;
                }
                else
                {
                    if (current_process.state == 0)
                    {
                        current_process.start_time = clock();
                        current_process.state = 1;
                    }
                    kill(current_process.pid, SIGCONT);

                    usleep(1000 * shared_mem->TSLICE);
                    kill(current_process.pid, SIGSTOP);

                    int stat = kill(current_process.pid, 0);
                    if (stat == 0)
                    {
                        enqueue2(currqueue, current_process);
                        usleep(1000);
                    }
                    else
                    {
                        kill(current_process.pid, SIGKILL);
                        double time_taken = (double)(clock() - current_process.start_time) / CLOCKS_PER_SEC;
                        addhistory(current_process.name, time_taken, current_process.pid);
                    }
                }
            }
        }
        k++;
    }

    return 0;
}
