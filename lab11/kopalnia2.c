#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define MAX_LINE_LENGTH 256
#define MAX_ROBOTS 6
#define MAX_QUEUE 100 

typedef struct {
    int cart_number;
    char stone_type[32];
    int weight;
    int quantity;
    int unload_time;   
} Wheelbarrow;

typedef struct {
    Wheelbarrow queue[MAX_QUEUE];
    int front, rear;  
} FIFOQueue;

typedef struct {
    Wheelbarrow current;  
    int is_working;       
    int remaining_time;   
    int time_spent;       
} Robot;

void init_queue(FIFOQueue *q) {
    q->front = 0;
    q->rear = 0;
}

int is_queue_empty(FIFOQueue *q) {
    return q->front == q->rear;
}

int is_queue_full(FIFOQueue *q) {
    return (q->rear + 1) % MAX_QUEUE == q->front;
}

void enqueue(FIFOQueue *q, Wheelbarrow wb) {
    if (is_queue_full(q)) {
        fprintf(stderr, "Queue is full. Cannot enqueue.\n");
        return;
    }
    q->queue[q->rear] = wb;
    q->rear = (q->rear + 1) % MAX_QUEUE;
}

Wheelbarrow dequeue(FIFOQueue *q) {
    if (is_queue_empty(q)) {
        fprintf(stderr, "Queue is empty. Cannot dequeue.\n");
        exit(EXIT_FAILURE);
    }
    Wheelbarrow wb = q->queue[q->front];
    q->front = (q->front + 1) % MAX_QUEUE;
    return wb;
}

void print_queue(FIFOQueue *queue) {
    if (is_queue_empty(queue)) {
        printf("Queue is empty.\n");
        return;
    }

    printf("Current queue state:\n");
    int i = queue->front;
    while (i != queue->rear) {
        Wheelbarrow wb = queue->queue[i];
        printf("[%d %s %d] ", wb.cart_number, wb.stone_type, wb.unload_time);
        i = (i + 1) % MAX_QUEUE;
    }
    printf("\n");
}

void print_robot_state(Robot robots[], int num_robots) {
    printf("                ");
    for (int i = 0; i < num_robots; i++) {
        if (robots[i].is_working) {
            printf("[%s %10d]", robots[i].current.stone_type, robots[i].remaining_time);
        } else {
            printf("[%13s]", " ");
        }
    }
    printf("\n");
}

void process_line(const char *line, FIFOQueue *queue, int *current_time, int *is_processing_started) {
    char buffer[MAX_LINE_LENGTH];
    strncpy(buffer, line, MAX_LINE_LENGTH);
    buffer[MAX_LINE_LENGTH - 1] = '\0';

    char *token = strtok(buffer, " ");
    int minute = atoi(token);

    if (!*is_processing_started) {
        *current_time = minute;
        printf("\nMoment %d\n", minute);
        *is_processing_started = 1;
    }

    if (minute != *current_time) {
        return;  
    }

    while ((token = strtok(NULL, " ")) != NULL) {
        Wheelbarrow wb;
        wb.cart_number = atoi(token);

        token = strtok(NULL, " ");
        strncpy(wb.stone_type, token, sizeof(wb.stone_type));
        wb.stone_type[sizeof(wb.stone_type) - 1] = '\0';

        token = strtok(NULL, " ");
        wb.weight = atoi(token);

        token = strtok(NULL, " ");
        wb.quantity = atoi(token);

        wb.unload_time = wb.weight * wb.quantity;

        printf("        wheelbarrow arrived <%d %s %d %d [%d]>\n",
               wb.cart_number, wb.stone_type, wb.weight, wb.quantity, wb.unload_time);

        enqueue(queue, wb);
    }
}

void update_robots_fcfs(Robot robots[], int num_robots, FIFOQueue *queue) {
    for (int i = 0; i < num_robots; i++) {
        if (robots[i].is_working) {
            robots[i].remaining_time--;
            if (robots[i].remaining_time == 0) {
                robots[i].is_working = 0;
            }
        }
    }

    for (int i = 0; i < num_robots; i++) {
        if (!robots[i].is_working && !is_queue_empty(queue)) {
            robots[i].current = dequeue(queue);
            robots[i].remaining_time = robots[i].current.unload_time;
            robots[i].is_working = 1;
        }
    }
}

void update_robots_rr(Robot robots[], int num_robots, FIFOQueue *queue, int time_quantum) {
    for (int i = 0; i < num_robots; i++) {
        if (robots[i].is_working) {
            robots[i].remaining_time--;
            robots[i].time_spent++;

            if (robots[i].remaining_time == 0) {
                robots[i].is_working = 0;
            } else if (robots[i].time_spent == time_quantum) {
                robots[i].time_spent = 0;
                Wheelbarrow updated_wb = robots[i].current;
                updated_wb.unload_time = robots[i].remaining_time;
                enqueue(queue, updated_wb);
                robots[i].is_working = 0;
            }
        }
    }

    for (int i = 0; i < num_robots; i++) {
        if (!robots[i].is_working && !is_queue_empty(queue)) {
            robots[i].current = dequeue(queue);
            robots[i].remaining_time = robots[i].current.unload_time;
            robots[i].is_working = 1;
            robots[i].time_spent = 0;
        }
    }
}

void update_robots_srtf(Robot robots[], int num_robots, FIFOQueue *queue) {
    for (int i = 0; i < num_robots; i++) {
        if (robots[i].is_working) {
            robots[i].remaining_time--;
            if (robots[i].remaining_time == 0) {
                robots[i].is_working = 0;
            }
        }
    }

    for (int i = 0; i < num_robots; i++) {
        if (robots[i].is_working) {
            int shortest_index = -1;
            int shortest_time = robots[i].remaining_time;

            for (int j = queue->front; j != queue->rear; j = (j + 1) % MAX_QUEUE) {
                if (queue->queue[j].unload_time < shortest_time) {
                    shortest_index = j;
                    shortest_time = queue->queue[j].unload_time;
                }
            }

            if (shortest_index != -1) {
                Wheelbarrow interrupted_task = robots[i].current;
                interrupted_task.unload_time = robots[i].remaining_time;
                enqueue(queue, interrupted_task);

                robots[i].current = queue->queue[shortest_index];
                robots[i].remaining_time = robots[i].current.unload_time;

                for (int k = shortest_index; k != queue->rear; k = (k + 1) % MAX_QUEUE) {
                    int next_index = (k + 1) % MAX_QUEUE;
                    queue->queue[k] = queue->queue[next_index];
                }
                queue->rear = (queue->rear - 1 + MAX_QUEUE) % MAX_QUEUE;

                robots[i].is_working = 1;
            }
        } else if (!is_queue_empty(queue)) {
            int shortest_index = queue->front;
            for (int j = queue->front; j != queue->rear; j = (j + 1) % MAX_QUEUE) {
                if (queue->queue[j].unload_time < queue->queue[shortest_index].unload_time) {
                    shortest_index = j;
                }
            }

            robots[i].current = queue->queue[shortest_index];
            robots[i].remaining_time = robots[i].current.unload_time;

            for (int k = shortest_index; k != queue->rear; k = (k + 1) % MAX_QUEUE) {
                int next_index = (k + 1) % MAX_QUEUE;
                queue->queue[k] = queue->queue[next_index];
            }
            queue->rear = (queue->rear - 1 + MAX_QUEUE) % MAX_QUEUE;

            robots[i].is_working = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <number_of_robots> <time_quantum> <input_file> <scheduling_algorithm>\n", argv[0]);
        return -1;
    }

    int num_robots = atoi(argv[1]);
    int time_quantum = atoi(argv[2]);
    const char *input_file = argv[3];
    int scheduling_algorithm = atoi(argv[4]);

    if (num_robots <= 0 || num_robots > MAX_ROBOTS) {
        fprintf(stderr, "Invalid number of robots. Must be between 1 and %d.\n", MAX_ROBOTS);
        return -1;
    }

    if (time_quantum < 1) {
        fprintf(stderr, "Invalid time quantum. Must be at least 1.\n");
        return -1;
    }

    FILE *file = NULL;
    if (strcmp(input_file, "-") == 0) {
        file = stdin;
    } else {
        file = fopen(input_file, "r");
        if (file == NULL) {
            perror("Error opening file");
            return -1;
        }
    }

    char line[MAX_LINE_LENGTH];

    Robot robots[MAX_ROBOTS] = {0};
    FIFOQueue queue;    
    init_queue(&queue);

    int current_time = 0;
    int all_done = 0;
    int is_processing_started = 0;

    printf("%d robots in the mine\n", num_robots);
    if (scheduling_algorithm == 1) {
        printf("\nRR with quantum %d:\n", time_quantum);
    } else if (scheduling_algorithm == 2) {
        printf("\nFCFS:\n");
    } else if (scheduling_algorithm == 3) {
        printf("\nSRTF:\n");
    } else {
        fprintf(stderr, "Invalid scheduling algorithm. Use 1 (RR), 2 (FCFS), or 3 (SRTF).\n");
        return -1;
    }

    while (1) {
        if (is_processing_started) {
            printf("\nMoment %d\n", current_time); 
        }

        if (!all_done && fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            process_line(line, &queue, &current_time, &is_processing_started);
        }

        if (is_processing_started) {
            if (scheduling_algorithm == 1) {
                update_robots_rr(robots, num_robots, &queue, time_quantum);
            } else if (scheduling_algorithm == 2) {
                update_robots_fcfs(robots, num_robots, &queue);
            } else if (scheduling_algorithm == 3) {
                update_robots_srtf(robots, num_robots, &queue);
            }
            print_robot_state(robots, num_robots);
    sleep(1);
           // print_queue(&queue);
        }

        all_done = is_queue_empty(&queue);
        for (int i = 0; i < num_robots; i++) {
            if (robots[i].is_working && robots[i].remaining_time > 1) {
                all_done = 0;
                break;
            }
        }

        if (all_done) {
            break;
        }

        current_time++;
    }

    if (file != stdin) {
        fclose(file);
    }

    printf("OK\n");
    return 0;
}
