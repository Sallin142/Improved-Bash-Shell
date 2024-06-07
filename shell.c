#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/sysinfo.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10
#define PROMPT_LENGTH (COMMAND_LENGTH + 256) // Increased buffer size for prompt

char history[HISTORY_DEPTH][COMMAND_LENGTH];
int history_count = 0;
char previous_dir[COMMAND_LENGTH];
char home_dir[COMMAND_LENGTH];
volatile sig_atomic_t sigint_received = 0;

// Function to concatenate command and its parameters into a single string
void concatenate_command(char *dest, char *tokens[]) {
    dest[0] = '\0';
    for (int i = 0; tokens[i] != NULL; i++) {
        strcat(dest, tokens[i]);
        if (tokens[i + 1] != NULL) {
            strcat(dest, " ");
        }
    }
}

// Function to add a command to the history
void add_to_history(char *tokens[]) {
    int index = history_count % HISTORY_DEPTH;
    char command_with_params[COMMAND_LENGTH] = {0};
    concatenate_command(command_with_params, tokens);
    strncpy(history[index], command_with_params, COMMAND_LENGTH - 1);
    history[index][COMMAND_LENGTH - 1] = '\0'; // Ensure null termination
    history_count++;
}

// Function to print the last 10 commands from the history
void print_history() {
    int end = history_count;
    int start = history_count > HISTORY_DEPTH ? history_count - HISTORY_DEPTH : 0;
    for (int i = end - 1; i >= start; i--) {
        int index = i % HISTORY_DEPTH;
        printf("%d\t%s\n", i, history[index]);
    }
}

// Function to retrieve a command from history
const char* get_command_from_history(int n) {
    if (n < 0 || n >= history_count || (history_count > HISTORY_DEPTH && n < history_count - HISTORY_DEPTH)) {
        return NULL;
    }
    return history[n % HISTORY_DEPTH];
}

// Function to clear the history
void clear_history() {
    history_count = 0;
    memset(history, 0, sizeof(history));
}

// Signal handler for SIGINT
void handle_sigint(int sig) {
    sigint_received = 1;
}

// Function to display help information
void display_help() {
    write(STDOUT_FILENO, "\nInternal commands:\n", 19);
    write(STDOUT_FILENO, "exit: Exit the shell program. If any argument is provided, an error is displayed.\n", 81);
    write(STDOUT_FILENO, "pwd: Display the current working directory. Takes no arguments.\n", 65);
    write(STDOUT_FILENO, "cd: Change the current working directory. Accepts one argument (absolute or relative path).\n", 93);
    write(STDOUT_FILENO, "help: Display this help information.\n", 36);
    write(STDOUT_FILENO, "history: Show command history. Displays the last 10 commands executed.\n", 71);
    write(STDOUT_FILENO, "!!: Repeat the last command executed.\n", 38);
    write(STDOUT_FILENO, "!n: Repeat the nth command from the history.\n", 46);
    write(STDOUT_FILENO, "!-: Clear the command history.\n", 31);

    // Re-display the command prompt
    char cwd[COMMAND_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "$ ", 2);
    } else {
        perror("getcwd() error");
    }
}

// Function to display detailed help for a specific internal command
void display_command_help(char *command) {
    if (strcmp(command, "exit") == 0) {
        write(STDOUT_FILENO, "'exit' is a builtin command for exiting the shell.\n", 52);
    } else if (strcmp(command, "pwd") == 0) {
        write(STDOUT_FILENO, "'pwd' is a builtin command for printing the current working directory.\n", 69);
    } else if (strcmp(command, "cd") == 0) {
        write(STDOUT_FILENO, "'cd' is a builtin command for changing the current working directory.\n", 69);
    } else if (strcmp(command, "help") == 0) {
        write(STDOUT_FILENO, "'help' is a builtin command for displaying this help message.\n", 61);
    } else if (strcmp(command, "history") == 0) {
        write(STDOUT_FILENO, "'history' is a builtin command for displaying the command history.\n", 67);
    } else if (strcmp(command, "!!") == 0) {
        write(STDOUT_FILENO, "'!!' is a builtin command for repeating the last command executed.\n", 65);
    } else if (command[0] == '!' && isdigit(command[1])) {
        write(STDOUT_FILENO, "'!n' is a builtin command for repeating the nth command from the history.\n", 74);
    } else if (strcmp(command, "!-") == 0) {
        write(STDOUT_FILENO, "'!-' is a builtin command for clearing the command history.\n", 61);
    } else {
        write(STDOUT_FILENO, command, strlen(command));
        write(STDOUT_FILENO, " is an external command or application\n", 39);
    }
}

// Function to read a command from the user
void read_command(char *buff, char *tokens[], _Bool *in_background) {
    int length = read(STDIN_FILENO, buff, COMMAND_LENGTH - 1);

    if (length < 0 && errno != EINTR) {
        perror("Unable to read command. Terminating.\n");
        exit(-1);
    } else if (length < 0 && errno == EINTR) {
        // Interrupted by a signal, just return
        return;
    }

    buff[length] = '\0';

    int token_count = 0;
    char *token = strtok(buff, " \t\n");
    while (token != NULL) {
        tokens[token_count++] = token;
        token = strtok(NULL, " \t\n");
    }
    tokens[token_count] = NULL;

    if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
        *in_background = true;
        tokens[token_count - 1] = NULL;
    } else {
        *in_background = false;
    }
}

// Function to handle internal commands
_Bool handle_internal_commands(char *tokens[]) {
    if (strcmp(tokens[0], "exit") == 0) {
        if (tokens[1] != NULL) {
            write(STDERR_FILENO, "exit does not take any arguments\n", 33);
        } else {
            exit(EXIT_SUCCESS);
        }
        return true;
    } else if (strcmp(tokens[0], "pwd") == 0) {
        if (tokens[1] != NULL) {
            write(STDERR_FILENO, "pwd does not take any arguments\n", 32);
        } else {
            char cwd[COMMAND_LENGTH];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                write(STDOUT_FILENO, cwd, strlen(cwd));
                write(STDOUT_FILENO, "\n", 1);
            } else {
                perror("getcwd() error");
            }
        }
        return true;
    } else if (strcmp(tokens[0], "cd") == 0) {
        if (tokens[1] == NULL) {
            // Change to the home directory if no argument is provided
            getcwd(previous_dir, sizeof(previous_dir)); // Save current directory
            if (chdir(home_dir) != 0) {
                perror("chdir() error");
            }
        } else if (tokens[2] != NULL) {
            write(STDERR_FILENO, "cd takes exactly one argument\n", 30);
        } else if (strcmp(tokens[1], "~") == 0) {
            // Change to the home directory if "~" is provided
            getcwd(previous_dir, sizeof(previous_dir)); // Save current directory
            if (chdir(home_dir) != 0) {
                perror("chdir() error");
            }
        } else if (strcmp(tokens[1], "-") == 0) {
            // Change to the previous directory if "-" is provided
            char temp_dir[COMMAND_LENGTH];
            getcwd(temp_dir, sizeof(temp_dir));
            if (chdir(previous_dir) == 0) {
                strcpy(previous_dir, temp_dir);
            } else {
                perror("chdir() error");
            }
        } else if (strncmp(tokens[1], "~/", 2) == 0) {
            // Change to a directory under the home directory
            char new_dir[COMMAND_LENGTH];
            if (snprintf(new_dir, sizeof(new_dir), "%s/%s", home_dir, tokens[1] + 2) >= sizeof(new_dir)) {
                write(STDERR_FILENO, "Path too long\n", 14);
            } else {
                getcwd(previous_dir, sizeof(previous_dir)); // Save current directory
                if (chdir(new_dir) != 0) {
                    perror("chdir() error");
                }
            }
        } else {
            // Change to the specified directory
            getcwd(previous_dir, sizeof(previous_dir)); // Save current directory
            if (chdir(tokens[1]) != 0) {
                perror("chdir() error");
            }
        }
        return true;
    } else if (strcmp(tokens[0], "help") == 0) {
        if (tokens[1] == NULL) {
            display_help();
        } else if (tokens[2] != NULL) {
            write(STDERR_FILENO, "help takes at most one argument\n", 32);
        } else {
            display_command_help(tokens[1]);
        }
        return true;
    }
    return false;
}

// Function to handle history-related commands
_Bool handle_history_command(char *tokens[], char *input_buffer, _Bool *in_background) {
    const char *command = NULL;

    if (tokens[0][1] == '!') {
        if (history_count == 0) {
            write(STDERR_FILENO, "No commands in history\n", 23);
            return false;
        }
        command = get_command_from_history(history_count - 1);
    } else if (tokens[0][1] == '-') {
        clear_history();
        write(STDOUT_FILENO, "History cleared\n", 16);
        return false;
    } else {
        int n = atoi(&tokens[0][1]);
        if (n == 0 && strcmp(&tokens[0][1], "0") != 0) {
            write(STDERR_FILENO, "Invalid command number\n", 23);
            return false;
        }
        command = get_command_from_history(n);
        if (command == NULL) {
            write(STDERR_FILENO, "Invalid command number\n", 23);
            return false;
        }
    }

    strncpy(input_buffer, command, COMMAND_LENGTH - 1);
    input_buffer[COMMAND_LENGTH - 1] = '\0';

    // Tokenize the input_buffer to update tokens array
    int token_count = 0;
    char *token = strtok(input_buffer, " \t\n");
    while (token != NULL) {
        tokens[token_count++] = token;
        token = strtok(NULL, " \t\n");
    }
    tokens[token_count] = NULL;

    write(STDOUT_FILENO, command, strlen(command));
    write(STDOUT_FILENO, "\n", 1);

    // Check if the resolved command is "history"
    if (strcmp(tokens[0], "history") == 0) {
        add_to_history(tokens);
        print_history();
        return false; // Do not execute further, as "history" is an internal command
    }

    return true;
}

// Function to get CPU usage
void get_cpu_usage(char *cpu_usage_str) {
    FILE *fp;
    char buffer[128];
    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/stat");
        strcpy(cpu_usage_str, "CPU: N/A");
        return;
    }
    fgets(buffer, sizeof(buffer) - 1, fp);
    fclose(fp);

    unsigned long user, nice, system, idle;
    sscanf(buffer, "cpu  %lu %lu %lu %lu", &user, &nice, &system, &idle);
    unsigned long total = user + nice + system + idle;

    static unsigned long prev_total = 0, prev_idle = 0;
    unsigned long delta_total = total - prev_total;
    unsigned long delta_idle = idle - prev_idle;

    if (delta_total == 0) {
        strcpy(cpu_usage_str, "CPU: N/A");
    } else {
        snprintf(cpu_usage_str, 64, "CPU: %.2f%%", 100.0 * (delta_total - delta_idle) / delta_total);
    }

    prev_total = total;
    prev_idle = idle;
}

// Function to get memory usage
void get_memory_usage(char *memory_usage_str) {
    struct sysinfo mem_info;
    sysinfo(&mem_info);
    unsigned long total_mem = mem_info.totalram / 1024 / 1024;
    unsigned long used_mem = (mem_info.totalram - mem_info.freeram) / 1024 / 1024;
    snprintf(memory_usage_str, 64, "Mem: %lu/%luMB", used_mem, total_mem);
}

// Function to display the prompt with system stats
void display_prompt() {
    char cwd[COMMAND_LENGTH];
    char cpu_usage[64];
    char memory_usage[64];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        get_cpu_usage(cpu_usage);
        get_memory_usage(memory_usage);
        char prompt[PROMPT_LENGTH];
        snprintf(prompt, sizeof(prompt), "%s [%s, %s]$ ", cwd, cpu_usage, memory_usage);
        write(STDOUT_FILENO, prompt, strlen(prompt));
    } else {
        perror("getcwd() error");
    }
}

int main(int argc, char* argv[]) {
    // Register the SIGINT signal handler
    signal(SIGINT, handle_sigint);

    // Get the home directory of the current user
    struct passwd *pw = getpwuid(getuid());
    strcpy(home_dir, pw->pw_dir);

    char input_buffer[COMMAND_LENGTH];
    char *tokens[NUM_TOKENS];
    getcwd(previous_dir, sizeof(previous_dir));

    while (true) {
        display_prompt();

        _Bool in_background = false;
        read_command(input_buffer, tokens, &in_background);

        if (sigint_received) {
            display_help();
            sigint_received = 0;
            continue;
        }

        if (tokens[0] == NULL) {
            continue;
        }

        if (strcmp(tokens[0], "history") == 0) {
            add_to_history(tokens);
            print_history();
            continue;
        } else if (tokens[0][0] == '!') {
            if (!handle_history_command(tokens, input_buffer, &in_background)) {
                continue;
            }
        }

        if (handle_internal_commands(tokens)) {
            add_to_history(tokens); // Add the original command to history
            continue;
        }

        add_to_history(tokens); // Add the original command to history
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(tokens[0], tokens) == -1) {
                write(STDERR_FILENO, "Command execution failed\n", 25);
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) {
            if (!in_background) {
                waitpid(pid, NULL, 0);
            }
        } else {
            write(STDERR_FILENO, "Fork failed\n", 12);
        }

        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    return 0;
}


