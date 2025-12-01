// things to do:
// differentiate between foreground nad background commands with $ at the end
// always have the current wd printed, use getcwd for this
// use forking to actually implement foreground vs background processes
// use msgs.h for the error messages
// execute the commands by tokenizing
// use waitpit to wait on any child process to catch zombies, this will require
// a loop use waitpit to return immediately if no child has exited use read()
// and write() for i/o, consult signal-safety first task: accept user input with
// read and write

#include <shell.h>

void internal_history(char *command) {

  char *copy = strdup(command);
  if (history_count < 10) {

    history[history_count++] = copy;
    return;
  }

  free(history[0]);
  // otherwise shift the array left
  for (int i = 0; i < HISTORY_SZ - 1; i++) {

    history[i] = history[i + 1];
  }

  // make the last element the current command
  history[HISTORY_SZ - 1] = copy;
  history_count++;
}

void print_number(int num) {

  char num_buf[12];
  int i = 11;

  if (num == 0) {

    write(STDOUT_FILENO, "0", 1);
    return;
  }

  while (num > 0 && i > 0) {

    num_buf[--i] = '0' + (num % 10);
    num /= 10;
  }

  write(STDOUT_FILENO, &num_buf[i], 11 - i);
}

void print_history() {

  if (history_count == 1) {

    const char *msg = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }

  int h = 0;

  if (history_count >= HISTORY_SZ) {

    h = history_count - HISTORY_SZ;
  }

  int min = (history_count) < (HISTORY_SZ) ? history_count : HISTORY_SZ;

  for (int i = min - 1; i >= 0; i--) {

    // print the current number
    print_number(i + h);
    write(STDOUT_FILENO, "\t", 1);
    write(STDOUT_FILENO, history[i], strlen(history[i]));

    int len = strlen(history[i]);
    if (history[i][len - 1] != '\n') {

      write(STDOUT_FILENO, "\n", 1);
    }
  }

  return;
}

void internal_exit(char *tokens[], int count) {

  if (count > 1) {

    const char *msg = FORMAT_MSG("exit", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }

  _exit(EXIT_SUCCESS);
}

void internal_pwd(char *tokens[], int count) {

  if (count > 1) {
    const char *msg = FORMAT_MSG("pwd", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  } else if (getcwd(cwd, PATH_MAX) == NULL) {
    // shell: unable to get current directory
    const char *msg = FORMAT_MSG("pwd", GETCWD_ERROR_MSG);
    write(STDERR_FILENO, msg,
          strlen(msg)); // needs to be msg's size, not the buffer size
                        //
    return;
  } // copes the cwd by iteslef, returns NULL on failure?
  else {
    write(STDOUT_FILENO, cwd, strlen(cwd));
    write(STDOUT_FILENO, "\n", 1);
  }
  return;
}

void internal_cd(char *tokens[], int count) {

  char target_dir[PATH_MAX];
  char current_dir[PATH_MAX];

  if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
    const char *msg = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  }

  if (count == 1) {

    struct passwd *pw = getpwuid(getuid());
    strcpy(target_dir, pw->pw_dir);

  }

  else if (count > 2) {

    const char *msg = FORMAT_MSG("cd", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;

  }

  else if (strcmp(tokens[1], "-") == 0) {

    strcpy(target_dir, prev_dir);
  }

  else if (tokens[1][0] == '~') { // first char of first argument

    struct passwd *pw = getpwuid(getuid());
    if (tokens[1][1] == '\0') {

      strcpy(target_dir, pw->pw_dir);

    } else if (tokens[1][1] == '/') {

      strcpy(target_dir, pw->pw_dir);
      strcat(target_dir, tokens[1] + 1);
    }
  }

  else {

    strcpy(target_dir, tokens[1]);
  }

  if (chdir(target_dir) == -1) {

    const char *msg = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  }

  strcpy(prev_dir, current_dir);

  return;
}

void internal_help(char *tokens[], int count) {

  if (count == 1) {

    // list all internal commands with their corresponding help message    //
    char *msg1 = FORMAT_MSG("cd", CD_HELP_MSG);
    write(STDOUT_FILENO, msg1, strlen(msg1));

    char *msg2 = FORMAT_MSG("pwd", PWD_HELP_MSG);
    write(STDOUT_FILENO, msg2, strlen(msg2));

    char *msg3 = FORMAT_MSG("exit", EXIT_HELP_MSG);
    write(STDOUT_FILENO, msg3, strlen(msg3));

    char *msg4 = FORMAT_MSG("history", HISTORY_HELP_MSG);
    write(STDOUT_FILENO, msg4, strlen(msg4));

    char *msg5 = FORMAT_MSG("help", HELP_HELP_MSG);
    write(STDOUT_FILENO, msg5, strlen(msg5));

    return;
  }

  else if (strcmp(tokens[1], "cd") == 0) {
    char *msg1 = FORMAT_MSG("cd", CD_HELP_MSG);
    write(STDOUT_FILENO, msg1, strlen(msg1));
    return;
  }

  else if (strcmp(tokens[1], "pwd") == 0) {

    char *msg2 = FORMAT_MSG("pwd", PWD_HELP_MSG);
    write(STDOUT_FILENO, msg2, strlen(msg2));
    return;
  }

  else if (strcmp(tokens[1], "exit") == 0) {

    char *msg3 = FORMAT_MSG("exit", EXIT_HELP_MSG);
    write(STDOUT_FILENO, msg3, strlen(msg3));

    return;
  }

  else if (strcmp(tokens[1], "history") == 0) {

    char *msg4 = FORMAT_MSG("history", HISTORY_HELP_MSG);
    write(STDOUT_FILENO, msg4, strlen(msg4));
    return;
  }

  else if (strcmp(tokens[1], "help") == 0) {

    char *msg5 = FORMAT_MSG("help", HELP_HELP_MSG);
    write(STDOUT_FILENO, msg5, strlen(msg5));
    return;

  }

  else {

    write(STDOUT_FILENO, tokens[1], strlen(tokens[0]));
    const char *msg = ": external command or application\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    return;
  }

  if (count > 1) {

    const char *msg = FORMAT_MSG("help", TMA_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }
}

int tokenize(char *tokens[], char cmd[]) {

  char *token = NULL;
  char *saveptr = NULL;
  char *str = cmd;
  char *delim = " \t\n";

  int count = 0;
  token = strtok_r(str, delim, &saveptr);
  while (token != NULL) {

    tokens[count] = token;
    token = strtok_r(NULL, delim, &saveptr);
    count++;
  }

  tokens[count] = NULL;
  return count;
}

void clean_zombies() {

  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
  }
}

void handler(int signo) {

  char *msg1 = FORMAT_MSG("\ncd", CD_HELP_MSG);
  write(STDOUT_FILENO, msg1, strlen(msg1));

  char *msg2 = FORMAT_MSG("pwd", PWD_HELP_MSG);
  write(STDOUT_FILENO, msg2, strlen(msg2));

  char *msg3 = FORMAT_MSG("exit", EXIT_HELP_MSG);
  write(STDOUT_FILENO, msg3, strlen(msg3));

  char *msg4 = FORMAT_MSG("history", HISTORY_HELP_MSG);
  write(STDOUT_FILENO, msg4, strlen(msg4));

  char *msg5 = FORMAT_MSG("help", HELP_HELP_MSG);
  write(STDOUT_FILENO, msg5, strlen(msg5));

  //  write(STDOUT_FILENO, cwd, strlen(cwd));
  // write(STDOUT_FILENO, "$", 1);

  return;
}

void print_prompt(void) {

  if (getcwd(cwd, PATH_MAX) == NULL) {

    // shell: unable to get current directory
    const char *msg = FORMAT_MSG("getcwd", GETCWD_ERROR_MSG);
    write(STDERR_FILENO, msg,
          strlen(msg)); // needs to be msg's size, not the buffer size
  } // copes the cwd by iteslef, returns NULL on failure?

  else {

    write(STDOUT_FILENO, cwd, strlen(cwd));
    write(STDOUT_FILENO, "$ ", 2);
  }
}

void handle_fork(char *tokens[], int count, int background) {

  pid_t pid = fork();
  if (pid == 0) {
    // child process
    if (execvp(tokens[0], tokens) == -1) {

      const char *msg = FORMAT_MSG("shell", EXEC_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      _exit(EXIT_FAILURE);
    }

  }

  else if (pid < 0) {
    // fork failed
    const char *msg = FORMAT_MSG("fork", FORK_ERROR_MSG); // if fork fails
    write(STDERR_FILENO, msg, strlen(msg));

  }

  else {

    // we're in the parent
    if (background != 1) {

      // if in foreground

      int status;

      if (waitpid(pid, &status, 0) == -1) {

        if (errno != EINTR) {
          const char *msg = FORMAT_MSG("wait", WAIT_ERROR_MSG);
          write(STDERR_FILENO, msg, strlen(msg));
        } // wait for child to terminate
      }
    }
  }
}

int is_valid_number(char *str) {

  if (str == NULL || *str == '\0')
    return 0;
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] < '0' || str[i] > '9')
      return 0;
  }
  return 1;
}

void history_argument(char *command) {

  char *cmd;

  if (command[0] == '!' && command[1] != '\0' && command[1] != '!') {

    if (!is_valid_number(command + 1)) {

      const char *msg = FORMAT_MSG("history", HISTORY_INVALID_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      return;
    }

    int n = atoi(command + 1);
    int start = history_count - 1;
    int end = history_count > 10 ? history_count - 10 : 0;

    if (n < end || n > start) {

      const char *msg = FORMAT_MSG("history", HISTORY_INVALID_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      return;
    }

    cmd = history[n];

  }

  else if (strcmp(command, "!!") == 0) {

    if (history_count == 0) {

      const char *msg = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      return;
    }

    cmd = history[history_count - 1];

  }

  else {

    const char *msg = FORMAT_MSG("history", HISTORY_INVALID_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  }

  internal_history(cmd);

  write(STDOUT_FILENO, cmd, strlen(cmd));

  char *tokens[64]; // should be big enough
  int count = tokenize(tokens, cmd);

  if (!handle_internals(tokens, count)) {

    int background = 0;
    if (strcmp(tokens[count - 1], "&") == 0) {

      background = 1;
      tokens[count - 1] = NULL;
    }

    handle_fork(tokens, count, background);
  }

  return;
}

int handle_internals(char *tokens[], int count) {

  if (strcmp(tokens[0], "exit") == 0) {

    internal_exit(tokens, count);
    return 1;
  }

  else if (strcmp(tokens[0], "pwd") == 0) {

    internal_pwd(tokens, count);
    return 1;
  }

  else if (strcmp(tokens[0], "cd") == 0) {

    internal_cd(tokens, count);
    return 1;
  }

  else if (strcmp(tokens[0], "help") == 0) {

    internal_help(tokens, count);
    return 1;
  } else if (strcmp(tokens[0], "history") == 0) {

    print_history();
    return 1;
  } else if (tokens[0][0] == '!') {

    history_argument(tokens[0]);
    return 1;
  }
  return 0;
}

int main() { // ssize_t read(int fd, void buf[.count], size_t count) int fd is
             // STDIN_FILENO = 0 because we're reading inpute so use
             // STDOUT_FILENO for write nevermind it's an ARRAY of the size we
             // pick but what's the buffer size? 4096 bytes? we will try
  // use while true loop to keep the input coming

  while (1) {

    clean_zombies(); // clean zombies before reading new commands
    // print the cwd first
    //
    //
    signal(SIGINT, handler);
    print_prompt();

    ssize_t n =
        read(STDIN_FILENO, buf, BUF_SZ); // read the command from the user

    if (n == -1) { // if the command cannot be read, give an error
                   //
      if (errno == EINTR) {

        // then read() got interrupted by ctrl-c
        // if works correctly, then the handler will get called
        continue;
      } else {
        // there was an actual error
        const char *msg = FORMAT_MSG("cmd", READ_ERROR_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
      }
    }

    // read the command from the useU

    // now, check whether the input is for foreground or background
    // and then makybe make another function to tokenize and actually call
    // commands?

    buf[n] = '\0';
    if (buf[n - 1] != '\n') {

      buf[n] = '\n';
      buf[n + 1] = '\0';
    }

    char rawline[BUF_SZ];
    strncpy(rawline, buf, BUF_SZ);

    char *tokens[64];
    int count = tokenize(tokens, buf);

    if (count == 0) {

      const char *msg = FORMAT_HISTORY("", HISTORY_NO_LAST_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      continue;
    }

    if (!(tokens[0][0] == '!')) {
      internal_history(rawline);
    }

    if (handle_internals(tokens, count))
      continue;

    int is_background = 0;
    if (strcmp(tokens[count - 1], "&") == 0) {

      is_background = 1;
      tokens[count - 1] = NULL;
    }

    handle_fork(tokens, count, is_background);
    // otherwise we don't wait, so don't need the else branch
  }
}
