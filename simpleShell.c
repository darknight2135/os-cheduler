#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/ipc.h>
typedef struct Process
{
    pid_t pid;
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

typedef struct
{
    Queue queue;
    int NPROCS;
    int TSLICE;
    char history[1024][1024];
    int historyCount;
} SharedMemory;
SharedMemory *shared_mem;

void child_handler2(int sig)
{
    int child_status;
    pid_t pid;
    while ((pid = waitpid(-1, &child_status, WNOHANG)) > 0)
    {
        printf("");
    }
}
const int MAX_SIZE = 1024;

char *commandHistory[1024];
void printHistory()
{
    printf("History:\n");
    for (int i = 0; i < shared_mem->historyCount; i++)
    {
        printf("%d: %s\n", i + 1, shared_mem->history[i]);
    }
}

void addhistory(char *command, double timeTaken, pid_t cpid)
{
    time_t currentTime = time(NULL);

    pid_t pid = cpid;
    char *commandCopy = strdup(command);
    commandHistory[shared_mem->historyCount] = commandCopy;
    char temp[1000];
    struct tm *timeInfo = localtime(&currentTime);
    sprintf(temp, "[%02d:%02d:%02d] PID: %d - %s (Time taken: %.6f seconds)",
            timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec,
            pid, command, timeTaken);
    strcpy(shared_mem->history[shared_mem->historyCount], temp);
    shared_mem->historyCount++;
}
void printHelp()
{
    printf("The following commands are supported:\n");
    printf("Basic Commands:\n");
    printf("  ls\n");
    printf("  echo\n");
    printf("  wc\n");
    printf("  grep\n");
    printf("  sort\n");
    printf("  uniq\n");
    printf("  cat\n");

    printf("System and Utility Commands:\n");
    printf("  history\n");
    printf("  sh\n");
    printf("  htop\n");
    printf("  pwd\n");
    printf("  sed\n");
    printf("  awk\n");

    printf("Exit and File Operations:\n");
    printf("  exit\n");
    printf("  mkdir\n");
    printf("  mv\n");
    printf("  cp\n");
    printf("  chmod\n");
    printf("  chown\n");
    printf("  chgrp\n");
    printf("  rm\n");

    printf("Additional Commands:\n");
    printf("  cal\n");
    printf("  whoami\n");
}
void printCommandHistory()
{
    for (int i = 0; i < shared_mem->historyCount; i++)
    {
        printf("%d: %s\n", i + 1, commandHistory[i]);
    }
}

const int commandSetLength = 26;
clock_t start_time, end_time;
double time_taken;

const int N = 1e5;

Queue createQueue(int capacity)
{
    Queue queue;

    queue.front = 0;
    queue.rear = -1;
    queue.capacity = capacity;
    return queue;
}

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

Process dequeue(Queue *queue)
{
    if (isQueueEmpty(queue))
    {
        Process nullPair = {-1, -1};
        return nullPair;
    }
    return queue->data[queue->front++];
}

void handleSignal(int signal)
{
    if (signal == SIGINT)
    {
        printf("\n");
        printHistory();
        // delete shared memory
        shm_unlink("my_shared_memory");
        munmap(shared_mem, sizeof(SharedMemory));

        exit(0);
    }
}
int main()
{
    signal(SIGCHLD, child_handler2);
    int sem_id = semget(IPC_PRIVATE, 1, 0666);
    if (sem_id == -1)
    {
        perror("Error creating semaphore");
        exit(EXIT_FAILURE);
    }
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
    shared_mem->queue = createQueue(N);
    shared_mem->historyCount = 0;
    int NPROCS, TSLICE;
    printf("Enter number of processes: ");
    scanf("%d", &NPROCS);
    printf("Enter Time Slice : ");
    scanf("%d", &TSLICE);
    char *commandset[] = {
        "ls",
        "echo",
        "wc",
        "grep",
        "sort",
        "uniq",
        "cat",
        "./",
        "history",
        "sh",
        "htop",
        "pwd",
        "help",
        "sed",
        "awk",
        "exit",
        "cp",
        "mv",
        "chmod",
        "chown",
        "chgrp",
        "mkdir",
        "rm",
        "cal",
        "whoami",
        "submit"};
    signal(SIGINT, handleSignal);
    pid_t ongoing_pid = -1;
    shared_mem->TSLICE = TSLICE;
    shared_mem->NPROCS = NPROCS;
    if (fork() == 0)
    {
        execlp("./simpleScheduler", "./simpleScheduler", NULL);
    }
    else
    {
        while (1)
        {
        start:
            int test = waitpid(-1, NULL, WNOHANG);
            fflush(stdout);
            pid_t pid;
            char input[MAX_SIZE];
            printf("SimpleShell$ ");
            fgets(input, sizeof(input), stdin);
            int temp = strcspn(input, "\n");
            input[temp] = '\0';
            if (input[0] == '\0')
            {
                continue;
            }
            char inputcopy[MAX_SIZE];
            strcpy(inputcopy, input);
            char *argv[2];
            int argc = 0;
            char *commands[10];
            int commandCount = 0;
            char *arg = strtok(input, "|");
            while (arg != NULL)
            {
                commands[commandCount++] = arg;
                arg = strtok(NULL, "|");
            }
            int pipes[commandCount - 1][2];
            for (int i = 0; i < commandCount - 1; i++)
            {
                if (pipe(pipes[i]) == -1)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }
            for (int i = 0; i < commandCount; i++)
            {
                waitpid(-1, NULL, WNOHANG);
                char *argv[10];
                int argc = 0;

                arg = strtok(commands[i], " ");
                while (arg != NULL)
                {
                    argv[argc++] = arg;
                    arg = strtok(NULL, " ");
                }
                argv[argc] = NULL;

                int com = -1;
                for (int j = 0; j < commandSetLength; j++)
                {
                    if (j == 7)
                    {
                        if (argv[0][0] == '.' && argv[0][1] == '/')
                        {
                            com = 7;
                            break;
                        }
                    }
                    if (strcmp(argv[0], commandset[j]) == 0)
                    {
                        com = j;
                        break;
                    }
                }
                start_time = clock();
                if (com == -1)
                {
                    end_time = clock();
                    time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                    addhistory(input, time_taken, getpid());
                    printf("Invalid Command\n");
                    break;
                }

                if (com == 8)
                {
                    printCommandHistory();
                    end_time = clock();
                    time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                    addhistory(input, time_taken, getpid());
                    continue;
                }
                if (com == 12)
                {
                    printHelp();
                    end_time = clock();
                    time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                    addhistory(input, time_taken, getpid());
                    continue;
                }
                if (com == 15)
                {
                    printHistory();
                    shm_unlink("my_shared_memory");
                    munmap(shared_mem, sizeof(SharedMemory));
                    exit(0);
                }
                if (com == 25)
                {
                    if (argc < 2)
                    {
                        // Insufficient arguments for the "submit" command
                        printf("Usage: submit <program_name> [priority]\n");
                        end_time = clock();
                        time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                        addhistory(input, time_taken, getpid());
                        continue;
                    }
                    if (argc > 3)
                    {
                        printf("Usage: submit <program_name> [priority]\n");
                        end_time = clock();
                        time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                        addhistory(input, time_taken, getpid());
                        continue;
                    }
                    else
                    {
                        Process process;
                        strncpy(process.name, argv[1], 50);
                        process.state = 0;
                        process.real = 1;
                        if (argv[2] == NULL)
                        {
                            process.priority = 1;
                        }
                        else
                        {
                            int priority = atoi(argv[2]);
                            if (priority >= 1 && priority <= 4)
                            {
                                process.priority = atoi(argv[2]);
                            }
                            else
                            {
                                printf("Invalid priority. Priority should be between 1 and 4\n");
                                end_time = clock();
                                time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                                addhistory(input, time_taken, getpid());
                                continue;
                            }
                        }
                        if (!(argv[1][0]=='.'&&argv[1][1]=='/')){
                            printf("Invalid program name. Program name should start with './'\n");
                            end_time = clock();
                            time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                            addhistory(input, time_taken, getpid());
                            continue;
                        }
                        pid_t pid = fork();
                        if (pid == -1)
                        {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        }
                        if (pid == 0)
                        {
                            execlp(argv[1], argv[1], NULL);
                            perror("execlp");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            waitpid(-1, NULL, WNOHANG);
                            process.pid = pid;
                            enqueue(&shared_mem->queue, process);
                        }
                    }

                    continue;
                }
                int andFlag = 0;
                int len = strlen(argv[argc - 1]);
                if (argv[argc - 1][len - 1] == '&')
                {
                    argv[argc - 1] = NULL;
                    andFlag = 1;
                }
                pid = fork();

                if (pid == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if (pid == 0)
                {
                    if (i > 0)
                    {
                        if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                        {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                        close(pipes[i - 1][0]);
                        close(pipes[i - 1][1]);
                    }

                    if (i < commandCount - 1)
                    {
                        if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                        {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                        close(pipes[i][0]);
                        close(pipes[i][1]);
                    }

                    (com == 7) ? execvp(argv[0], argv) : execvp(commandset[com], argv);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    if (i > 0)
                    {
                        close(pipes[i - 1][0]);
                        close(pipes[i - 1][1]);
                    }
                    if (!andFlag)
                    {
                        waitpid(pid, NULL, 0);
                    }
                    else
                    {
                        ongoing_pid = pid;
                        printf("\nSimpShell: pid: %d, \'%s\' is running in the background\n", pid, inputcopy);
                    }
                }

                end_time = clock();
                time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                addhistory(inputcopy, time_taken, pid);
            }
            waitpid(-1, NULL, WNOHANG);
            for (int i = 0; i < commandCount - 1; i++)
            {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
        }
    }
    return 0;
}
