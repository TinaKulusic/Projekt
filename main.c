#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define HISTORY_FILE "/home/ming/Desktop/history.txt"
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_time(char **args);
int lsh_print_hist(char **args);
int delete_all_history(char **args);
int run_prev_history(char **args);
int run_target_history(char **args, int bang_num);




/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "time",
  "history",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_time,
  &lsh_exit,
  &lsh_history
  
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}
int lsh_print_hist(char **args)
{
  char line[100][100];
  int x;
  int y = 0;
  int total = 0;
  int temp = 0;



  FILE * record = NULL;
  record = fopen(HISTORY_FILE, "r");  // open connection to history record

  if (record != NULL)  
  {
    fseek(record, 0, SEEK_END);
    x = ftell(record);  
  //  printf("%i", x);
  //  printf("%i", &x);
    if (x == 0)  
    {
      printf("History.txt file empty.  Please enter more commands.\n");
      return 1;
    }
  }

  fclose(record);  // close record stream
  free(record);  // deallocate record
  record = fopen(HISTORY_FILE, "r");  //

  while (fgets(line[y], 100, record))
  {
    line[y][strlen(line[y]) - 1] = 0;    // 'remove \n' 
    y++;
  }

  total = y;
  printf("\n");
  printf("****************************************************\n");
  for (y = 0; y < total; ++y)
  {
    printf("%d  ", y + 1);
    printf("%s", line[y]);
    printf("\n");
  }
  printf("****************************************************\n");
  printf("\n");
  fclose(record);  // close history file
  return 1;
}

// delete all entries in the history record
int delete_all_history(char **args)
{
  FILE *record;
  record = fopen(HISTORY_FILE, "w");
  fclose(record);
  return 1;
}
int run_prev_history(char **args)
{
  char line[100][100];
  char **temp;
  int x = 0;
  int y = 0;
  int total = 0;

  FILE *record = NULL;
  record = fopen(HISTORY_FILE, "r");
  while (fgets(line[x], 100, record))
  {
    line[x][strlen(line[x]) - 1] = 0;
    x++;
  }

  for (y = 0; y < builtin_func_count(); y++)
  {
    if (strcmp(builtin_str[y], line[x - 1]) == 0)
    {
      return (*builtin_func[y])(args);
    }
  }

  temp = split_line(line[x - 1]);
  fclose(record);
  return launch(temp);
}


// *******************************************************************************************

// use index to find target history and launch
int run_target_history(char **args, int index)
{

  FILE *record = NULL;
  int x = 0;
  int y = 0;
  char line[100][100];
  // int total = 0;
  char **temp;

  record = fopen(HISTORY_FILE, "r");
  while (fgets(line[x], 100, record) && x < index)
  {
    line[x][strlen(line[x]) - 1] = 0;
    x++;
  }
  for (y = 0; y < builtin_func_count(); y++)
  {
    if (strcmp(builtin_str[y], line[x - 1]) == 0)
    {
      return (builtin_func[y])(args);
    }
  }

  temp = split_line(line[x - 1]);
  fclose(record);
  return launch(temp);
}
int lsh_time(char **args)
{
  time_t mytime = time(NULL);
  char *time_str = ctime(&mytime);
  time_str[strlen(time_str) - 1] = 0;
  printf("Current time: %s", time_str);
  printf("\n");
  return 1;
}
/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args)
{
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

#define LSH_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
