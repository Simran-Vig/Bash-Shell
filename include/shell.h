
#define _POSIX_C_SOURCE 200809L
#include "msgs.h"
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH_MAX 4096
#define BUF_SZ 1024
#define HISTORY_SZ 10

char buf[BUF_SZ];
char cwd[PATH_MAX];
char prev_dir[PATH_MAX] = "";
char *history[HISTORY_SZ];
int history_count = 0;

int handle_internals(char *tokens[], int count);
void history_argument(char *command);
void internal_history(char *command);
void print_number(int num);
void print_history();
void internal_exit(char *tokens[], int count);
void internal_pwd(char *tokens[], int count);
void internal_cd(char *tokens[], int count);
void internal_help(char *tokens[], int count);
int tokenize(char *tokens[], char cmd[]);
void clean_zombies();
void handler(int signo);
void print_prompt(void);
void handle_fork(char *tokens[], int count, int background);
int is_valid_number(char *str);
void history_argument(char *command);
int handle_internals(char *tokens[], int count);
