//Current Implementation does not support flags
//Commands do not have restriction on the number of arguments

//Including Required Files
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

//Defining Required Variables
#define ByteShell_TOK_DELIM ' '
#define ByteShell_TOK_DELIM_str " "

//Declaring Variables to store Shell Launch Time
time_t start_time;
struct tm strttm;

//Function Prototypes
int ByteShell_cd(char** args);
int ByteShell_commandlist(char** args);
int ByteShell_echo(char** args);
int ByteShell_help(char** args);
int ByteShell_view_history(char** args);
int ByteShell_logout(char** args);
int ByteShell_pwd(char** args);

void add_to_hist(char* line);
void ByteShell_init();
int ByteShell_terminate();

char* ByteShell_read_line();
char** ByteShell_split_line(char* line);
int ByteShell_launch(char** args);
int ByteShell_execute_line(char** args);
void ByteShell_loop();


//Defining Linked List Nodes
struct Node {
    char* str;
    struct Node* next;
};

//Definig Nodes to Store History
struct Node* history_head = NULL;
struct Node* history_current = NULL;


//Storing Names and Pointers to Builtin Functions
int builtins_count = 7;

char* builtins[] = {
    "cd",
    "commandlist",
    "echo",
    "help",
    "history",
    "logout",
    "pwd"
};

char* builtin_description[] = {
    "change the present working directory",
    "show the available commands",
    "write arguments to the standard output",
    "return meaningful information about commands",
    "show current session history",
    "logout from current session",
    "return working directory name"
};

int (*builtin_func[]) (char **) = {
    &ByteShell_cd,
    &ByteShell_commandlist,
    &ByteShell_echo,
    &ByteShell_help,
    &ByteShell_view_history,
    &ByteShell_logout,
    &ByteShell_pwd
};

int ByteShell_cd(char** args) {

    /* Caller Command - "cd"
     * Used to change the present working directory of the user */
    
    if (args[1] == NULL) {
        fprintf(stderr, "ByteShell: Expected Argument to \"cd\"\n"); //Message if no argument is given to the cd command
    }
    else {
        //Change the directory
        if (chdir(args[1]) != 0) {
            perror("ByteShell"); //Error Message if the directory cannot be changed
        }
    }

    return 1;

}

int ByteShell_commandlist(char** args) {

    /* Caller Command - "commandlist"
     * Used to show the available commands */

    for (int i = 0; i < builtins_count; i++) {
        printf(" %s\n",builtins[i]);
    }

    return 1;

}

int ByteShell_echo(char** args) {

    /* Caller Command - "echo"
     * Used to print the text after the */

    int pos = 1;

    while (args[pos] != NULL) {
        printf("%s ",args[pos]);
        pos++;
    }
    printf("\n");

    return 1;
}

int ByteShell_help(char** args) {

    if (args[1] == NULL) {
        fprintf(stderr, "ByteShell: Expected Argument to \"help\"\n"); //Message if no argument is given to the cd command
    }
    else {
        for (int i = 0; i < builtins_count; i++) {
            if (strcmp(args[1], builtins[i]) == 0) {
                printf("%s - %s\n", args[1], builtin_description[i]);
                return 1;
            }
        }
        printf("Command \"%s\" not found\n", args[1]);
    }

    return 1;
}

int ByteShell_view_history(char** args) {

    /* Caller Command - "history"
     * Used to show the history */

    struct Node* ptr = history_head;
    int i = 1;

    while (ptr != NULL) {
        printf("%d.\t%s\n", i++, ptr->str);
        ptr = ptr->next;
    }

    free(ptr);

    return 1;

}

int ByteShell_logout(char** args) {

    /* Caller Command - "logout"
     * Used to logout */

    return ByteShell_terminate();

}

int ByteShell_pwd(char** args) {

    /* Caller Command - "pwd"
     * Shows current directory of the user */

    char* cwd = malloc(1024 * sizeof(char));
    getcwd(cwd, 1024 * sizeof(char));
    printf("%s\n", cwd); //Displaying Current Directory
    free(cwd);

    return 1;
}

void add_to_hist(char* line) {

    //Adding the present command to history

    //Defining node with data of current history
    struct Node* ptr = (struct Node*) malloc(sizeof (struct Node));
    ptr->str = (char*) malloc(sizeof(char)*1024);
    strcpy(ptr->str, line);

    ptr->next = NULL;

    //Adding the node to linked list
    
    if (history_head == NULL) {
        history_current = ptr;
        history_head = history_current;
    }
    else {
        history_current->next = ptr;
        history_current = ptr;
    }

}

// void init_history() {

//    This function can be written to initialize history files, so that each session can access history of previous sessions
//    and history would be written into a text file directly instead of the list
    
// }

void ByteShell_init() {

    //Function to Initialize the Shell

    // init_history();

    system("clear");
    char* cwd = malloc(1024 * sizeof(char));
    getcwd(cwd, 1024 * sizeof(char));
    printf("Current Dirctory: %s\n", cwd); //Displaying Current Directory
    free(cwd);

    start_time = time(NULL);
    strttm = *localtime(&start_time);

    printf("Last Login: %s\n", ctime(&start_time)); //Displaying Login Time

}

int ByteShell_terminate() {

    // Function to terminate the shell
    // Presently it is called only when the logout function is executed and not when the shell quits in other circumstances

    //Save History
    printf("Saving session...\n");

    time_t end_time;
    end_time = time(NULL);
    struct tm endtm = *localtime(&end_time);

    //Creating history logs file
    FILE* historyfile;
    char* historyfile_name = malloc(100*sizeof(char));

    if (!historyfile_name) {
        fprintf(stderr, "ByteShell: Allocation Error\n");
        exit(EXIT_FAILURE);
    }

    snprintf(historyfile_name, 100*sizeof(char), "ByteShellLogs_%d-%d-%d_%d-%d-%d.txt", endtm.tm_mday, endtm.tm_mon + 1, endtm.tm_year+1900, endtm.tm_hour, endtm.tm_min, endtm.tm_sec);
    
    historyfile = fopen(historyfile_name, "w");

    if (historyfile == NULL) {
        printf("History cannot be saved\n");
        printf("Logging out without saving\n");
        return EXIT_FAILURE;
    }

    //Writing session details into file

    printf("...saving history...");

    fprintf(historyfile, "Session Details\n");
    fprintf(historyfile, "Last Login: %s", ctime(&start_time));
    fprintf(historyfile, "Logout: %s", ctime(&end_time));

    //Writing history details into file

    struct Node* save_hist_node = history_head; //Node to traverse list of containing history of session
    int i = 1;

    printf("writing history files...");

    while (save_hist_node != NULL) {
        fprintf(historyfile, "%d.\t%s\n", i++, save_hist_node->str);
        save_hist_node = save_hist_node->next;
    }

    printf("saving history files...\n");

    //Closing history file
    fclose(historyfile);
    printf("...completed.\n");

    return EXIT_SUCCESS;

}

char* ByteShell_read_line() {

    //Function to read input from user
    
    int bufsize = 1024;
    int ByteShell_RL_BUFFSIZE = 512;
    int position = 0;
    int argcount = 0; 
    char* buffer = malloc(sizeof(char) * bufsize);
    char c;

    if (!buffer) {
        fprintf(stderr, "ByteShell: Allocation Error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {

        c = getchar(); //Getting character from input stream

        if (c == '\n' && position == 0) {
            return "0";
        }
        
        if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        
        buffer[position] = c;
        position++;

        if (position >= bufsize) {
            bufsize += ByteShell_RL_BUFFSIZE;
            buffer = realloc(buffer, bufsize);
        }

        if (!buffer) {
            fprintf(stderr, "ByteShell: Allocation Error\n");
            exit(EXIT_FAILURE);
        }

    }

}

char** ByteShell_split_line(char* line) {

    //Function to split input into argument tokens

    int bufsize = 64;
    int position = 0;
    int ByteShell_RL_BUFFSIZE = 32;
    char** tokens = malloc(bufsize * sizeof(char *));
    char* token;

    if (!tokens) {
        fprintf(stderr, "ByteShell: Allocation Error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, ByteShell_TOK_DELIM_str);

    do {

        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += ByteShell_RL_BUFFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
        }

        if (!tokens) {
            fprintf(stderr, "ByteShell: Allocation Error\n");
            exit(EXIT_FAILURE);
        }

        token = strtok(NULL, ByteShell_TOK_DELIM_str);

    } while (token != NULL);

    return tokens;

}

int ByteShell_launch(char** args) {

    // Function to Launch a Child Thread

    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {

        if (execvp(args[0], args) == -1) {
            perror("ByteShell");
        }

        exit(EXIT_FAILURE);

    }
    else {

        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        return 1;

    }

}

int ByteShell_execute_line(char** args) {

    // Function to Execute Commands

    if (args[0] == NULL) {
        return 1;
    }
    
    //Searching for the required builtin
    for (int i = 0; i < sizeof(builtins)/sizeof(char*); i++) {

        if (strcmp(args[0], builtins[i]) == 0) {
            return (*builtin_func[i])(args);
        }

    }

    return ByteShell_launch(args); //Launching a child thread

}

void ByteShell_loop() {

    // Basic Shell Loop

    char* line = "0"; //Variable to store command entered
    char** args; //Variable to store arguments
    int status = 0; //Status value for the Shell

    do {

        printf(">"); //Shell Prompt
        line = ByteShell_read_line(); //Reading the command
        if (line[0] != '0') {
            add_to_hist(line); //Adding the command to history
            args = ByteShell_split_line(line);//Obtaining the necessart commands 
            status = ByteShell_execute_line(args); //Executing the commands

            // Freeing the memory occupied by variables
            free(line);
            free(args);
        }
    
    } while (status);

}

int main(int argc, char**argv) {

    ByteShell_init(); //Initializing the shell

    ByteShell_loop(); //Shell loop

    return EXIT_SUCCESS;

}