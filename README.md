# SimpleScheduler (Group-19) 

## Contributions

### Dikshant Agarwal
- Working of Round Robin , Error Handling , make file

### Saksham Kapoor
- Shared Memory , History , Readme

## Implementation Overview

The provided code is an implementation of a scheduler within a shell that utilizes a round-robin scheduling algorithm.

## Key Features

1. **Input Parameters**: The code accepts two important input parameters:
   - `TSLICE`: This parameter represents the time quantum assigned to each process.
   - `NPROCS`: The maximum number of processes that can be scheduled.

2. **Shared Memory**: Shared memory is employed to facilitate communication and data sharing between the shell and the scheduler. Key components of shared memory include:
   - `NPROC`: This variable holds the number of processes.
   - `TSLICE`: It stores the time quantum.
   - `Queue`: A max priority queue manages the priority of processes to be executed.
   - `History`: A feature to maintain a history of executed processes.

3. **Round Robin Scheduling**: 
    1.Each process in the queue is assigned the same priority or time slice by default that is given the same priority.

    2.The processes are scheduled in a circular order, with each process running for its time slice before moving to the next one. If a process's time slice expires, it's placed at the end of the queue to wait for its next turn.


## How to run

1. Clone the repository
2. Run `make` to compile the code
3. Run `make run` to start the shell

### Source :
Github Link https://github.com/darknight2135/Scheduler