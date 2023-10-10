#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>

char **cmdArgs; 
int numArgs; 
int numBackgroundProcesses = 0; 
int array[20];
int currprocPid;
pid_t shellPid; 
int isPipe; 
char * pipes[10][100]; 

struct BackgroundProcess {
    int id;
    int procArgs; 
    pid_t pid; 
    pid_t parentPid; 
    char status[10]; 
    char name[50];
    char* args[20];

};

typedef struct {
    int ID;
    char **arguments; // This will be an array of strings (char pointers)
} PipedProcesses;

struct PipedProcesses * processes = NULL; 
struct BackgroundProcess * backgroundProcesses = NULL; 


int printJobs() {
  for (int i=0; i< numBackgroundProcesses; i++ ){
    //<id>: <program name> <arg1> <arg2> … <argN> [&]
    if (strcmp(backgroundProcesses[i].status, "BG") == 0){
    char buffer[256]; 
    sprintf(buffer, "%d: %s", backgroundProcesses[i].id, backgroundProcesses[i].name);
    for (int j = 0; j < backgroundProcesses[i].procArgs; j++){
      strcat(buffer, " "); 
      if (strcmp(backgroundProcesses[i].args[j], "&") == 0){ 
        strcat(buffer, "[&]");
      }
      else {
        strcat(buffer, backgroundProcesses[i].args[j]); 
      }

    }
    printf("%s\n", buffer); 
    }
    if (strcmp(backgroundProcesses[i].status, "ST") == 0){
    char buffer[256]; 
    sprintf(buffer, "%d: %s", backgroundProcesses[i].id, backgroundProcesses[i].name);
    for (int j = 0; j < backgroundProcesses[i].procArgs; j++){
      strcat(buffer, " "); 
      strcat(buffer, backgroundProcesses[i].args[j]); 

    }
    printf("%s\n", buffer); 
    }
  }
  return 0; 
}

void removeProcessFromList(pid_t pid){
  for (int i=0; i< numBackgroundProcesses; i++ ){
    if (backgroundProcesses[i].pid == pid){
      // we need to remove this process 
      array[backgroundProcesses[i].id -1] = 0; 
      for (int j = i; j < numBackgroundProcesses - 1; j++) {
        backgroundProcesses[j] = backgroundProcesses[j + 1];
      }
      // decrement the process 
      numBackgroundProcesses--; 
      backgroundProcesses = (struct BackgroundProcess*)realloc(backgroundProcesses, (numBackgroundProcesses) * sizeof(struct BackgroundProcess));
      break; 
    }
  }
}


void child_handler(int signo) {
  if (signo == SIGCHLD) {
    //printf("does this happen\n");
    int status; 
    pid_t pid; 
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
      if (WIFEXITED(status)) {
        removeProcessFromList(pid); 
      }
    }
    //tcsetpgrp(STDIN_FILENO, getpgid(shellPid));
  }
}
void sigtstp_handler(int signo) {
  // I need to stop the current process 
  // suspend it
    for (int i=0; i< numBackgroundProcesses; i++ ){
        if (strcmp(backgroundProcesses[i].status, "FG") == 0){
          strcpy(backgroundProcesses[i].status, strdup("ST") );
          killpg(getpgid(backgroundProcesses[i].pid), SIGTSTP);
            break;
        }
    }
}
void sigint_handler(int signo) {
	for (int i=0; i< numBackgroundProcesses; i++ ){
		if (strcmp(backgroundProcesses[i].status, "FG") == 0){
		killpg(getpgid(backgroundProcesses[i].pid), SIGINT); 
		}
	}
}


void find_stopped_job(int valid){

  if (valid){
    for (int i=0; i< numBackgroundProcesses; i++ ){
      if (backgroundProcesses[i].id == valid && (strcmp(backgroundProcesses[i].status, "ST") == 0)){
        strcpy(backgroundProcesses[i].status, "BG" );
        killpg(getpgid(backgroundProcesses[i].pid), SIGCONT);

        tcsetpgrp(STDIN_FILENO, getpgid(backgroundProcesses[i].parentPid)); 
          break; 
        } 
      }
  }
  else {
    for (int i=numBackgroundProcesses-1; i>= 0; i-- ){
      
      if ((strcmp(backgroundProcesses[i].status, "ST") == 0)){
        strcpy(backgroundProcesses[i].status, "BG" );
        killpg(getpgid(backgroundProcesses[i].pid), SIGCONT);
        tcsetpgrp(STDIN_FILENO, getpgid(backgroundProcesses[i].parentPid)); 
          break; 
        } 
      } 
    }
  
}

void find_running_job(int valid){

  if (valid){
    for (int i=0; i< numBackgroundProcesses; i++ ){
      if (backgroundProcesses[i].id == valid && (strcmp(backgroundProcesses[i].status, "FG") != 0)){
        strcpy(backgroundProcesses[i].status, "FG" );
        killpg(getpgid(backgroundProcesses[i].pid), SIGCONT);
        waitpid(backgroundProcesses[i].pid, NULL, WUNTRACED);
          break; 
        } 
      }
  }
  else {
    for (int i=numBackgroundProcesses-1; i>= 0; i-- ){
      
      if ((strcmp(backgroundProcesses[i].status, "FG") != 0)){
        strcpy(backgroundProcesses[i].status, "FG" );
	     killpg(getpgid(backgroundProcesses[i].pid), SIGCONT);
        waitpid(backgroundProcesses[i].pid, NULL, WUNTRACED);
        break; 
        } 
      } 
    }
  
}

int execPipe() {
  int bg = 0;
  if (strcmp(cmdArgs[numArgs-1], "&")==0){
    bg = 1; 
  }

  numBackgroundProcesses++;
  struct BackgroundProcess* currBgProcess = malloc(sizeof(struct BackgroundProcess)); 
  if (currBgProcess == NULL){
    exit(1); 
  }
 
  if (backgroundProcesses == NULL){
    currBgProcess->id = 1;
    array[0] = 1; 
  }
  else {
    for (int i=0; i<numBackgroundProcesses-1; i++){
      int id = backgroundProcesses[i].id; 
      array[id-1]= id; 
    }
    for (int j =0; j<20; j++){ 
      if (array[j] == 0){
        currBgProcess->id = j+1; 
        break;
      }
    }
  }
  strcpy(currBgProcess->name, strdup(cmdArgs[0])); 

  currBgProcess->parentPid = getpid();
  int numProcArgs = 0; 
  for (int i = 1; i < numArgs && i < 10; i++) {
    numProcArgs++; 
    currBgProcess->args[i-1] = strdup(cmdArgs[i]);
  }
  currBgProcess->procArgs = numProcArgs; 

  if (!bg){
    strcpy(currBgProcess->status, strdup("FG"));
  }
  else {
    strcpy(currBgProcess->status, strdup("BG"));


  }
  backgroundProcesses = (struct BackgroundProcess*)realloc(backgroundProcesses, (numBackgroundProcesses) * sizeof(struct BackgroundProcess));
  int numPipes = 0;

  for(int i=0; i<numArgs; i++){
    if (strcmp(cmdArgs[i], "|") == 0){
      numPipes++;
    }
  }
  numPipes++;

  char* currentArray[100];
  int currentArrayPlace = 0;
  int pipesSize = 0;
  int numberPipesThrough = 0;

  for(int i = 0; i < numArgs; i++){
      if (strcmp(cmdArgs[i], "|") == 0 || (!cmdArgs[i])){
          numberPipesThrough++;

          for (int j = 0; j < currentArrayPlace; j++){
              pipes[pipesSize][j] = currentArray[j];
              currentArray[j] = NULL; // Set each pointer to NULL
          }
          currentArrayPlace = 0; 
          pipesSize++; 
      }
      else {
          if (strcmp(cmdArgs[i], "&") == 0 ){
            continue; 
          }
          currentArray[currentArrayPlace] = cmdArgs[i];
          currentArrayPlace++; 
      }
  }
  for (int j = 0; j < currentArrayPlace; j++){
    pipes[pipesSize][j] = currentArray[j];
  }
  
  int pipefdArray[numPipes-1][2];
  int pids[numPipes]; 
  pid_t pgid = -1; 

  // open all the pipes
  for (int i = 0; i < numPipes; i++) {
      if (pipe(pipefdArray[i]) == -1) {
          perror("pipe");
          return 1; 
      }
  }

  for (int i = 0; i < numPipes; i++) {
    pids[i] = fork();
    
    if (pids[i] == 0) {
     if (i == 0) {
        pgid = pids[0];
        setpgid(pids[0], pgid);
      }
      else {
       setpgid(pids[i], pgid);
      }
      if (i==0){
        close(pipefdArray[0][0]); 
        dup2(pipefdArray[0][1], STDOUT_FILENO);

        close(pipefdArray[0][1]);
        for (int i = 0; i < numPipes; i++) {
            close(pipefdArray[i][0]);
            close(pipefdArray[i][1]);
        }

      }

      else if (i== (numPipes-1)){
        close(pipefdArray[i-1][1]);
        dup2(pipefdArray[i-1][0], STDIN_FILENO);
        close(pipefdArray[i-1][0]);
        for (int i = 0; i < numPipes; i++) {
            close(pipefdArray[i][0]);
            close(pipefdArray[i][1]);
        }

      }

      else {
        dup2(pipefdArray[i-1][0], STDIN_FILENO);
        close(pipefdArray[i-1][0]);
        dup2(pipefdArray[i][1], STDOUT_FILENO);
        close(pipefdArray[i][1]);
        for (int i = 0; i < numPipes; i++) {
            close(pipefdArray[i][0]);
            close(pipefdArray[i][1]);
        }

      }
      int pipeArgLength = 0; 
      for (int k=0; pipes[i][k]; k++){
        pipeArgLength++; 
      }
      char *myargs[pipeArgLength+1];

      for (int l=0; l<pipeArgLength; l++){
        myargs[l] = pipes[i][l];
      }

      myargs[pipeArgLength] = NULL; // set last to null
      execvp(myargs[0], myargs); // exectue 
      perror("execvp");
      exit(1);
    } 
  }

  currBgProcess->pid = pids[0];
  backgroundProcesses[numBackgroundProcesses-1] = *currBgProcess;
  free(currBgProcess);

  for (int i = 0; i < numPipes; i++) {
      close(pipefdArray[i][0]);
      close(pipefdArray[i][1]);
  }

  if (!bg){
    for (int i = 0; i < numPipes; i++) {
      waitpid(pids[i], NULL, 0);
  }
  }
  if (strcmp(backgroundProcesses[numBackgroundProcesses-1].status, "FG") == 0){
    removeProcessFromList(pids[0]); 
  }
  signal(SIGCHLD,child_handler);

  isPipe = 0; 

  return 0;
}

int execCmd() {
  numBackgroundProcesses++;
  struct BackgroundProcess* currBgProcess = malloc(sizeof(struct BackgroundProcess)); 
  if (currBgProcess == NULL){
    exit(1); 
  }
 
  if (backgroundProcesses == NULL){
    currBgProcess->id = 1;
    array[0] = 1; 
  }
  else {
    for (int i=0; i<numBackgroundProcesses-1; i++){
      int id = backgroundProcesses[i].id; 
      array[id-1]= id; 
    }
    for (int j =0; j<20; j++){
      if (array[j] == 0){
        currBgProcess->id = j+1; 
        break;
      }
    }
  }
  strcpy(currBgProcess->name, strdup(cmdArgs[0]));
  currBgProcess->parentPid = getpid(); 
  // set up array of args
  int numProcArgs = 0;
  for (int i = 1; i < numArgs && i < 10; i++) {
    numProcArgs++; 
    currBgProcess->args[i-1] = strdup(cmdArgs[i]);
  }
  currBgProcess->procArgs = numProcArgs; 

  if (strcmp(cmdArgs[numArgs-1], "&") != 0 ){
    strcpy(currBgProcess->status, strdup("FG"));
    }
  else {
    strcpy(currBgProcess->status, strdup("BG"));
  }
  backgroundProcesses = (struct BackgroundProcess*)realloc(backgroundProcesses, (numBackgroundProcesses) * sizeof(struct BackgroundProcess));

  int rc = fork();

  if (rc < 0) {
      // fork failed; exit
      fprintf(stderr, "fork failed\n");
      exit(1);
  } 
  else if (rc == 0) {
      setpgid(0, 0);

      if (strcmp(cmdArgs[numArgs-1], "&") != 0 ){
        char *myargsfg[numArgs+1];
        for (int i=0; i<numArgs; i++ ){
          myargsfg[i] = strdup(cmdArgs[i]);

        }
        myargsfg[numArgs] = NULL;
        execvp(myargsfg[0], myargsfg); 
      }
      else {
        char *myargs[numArgs-1];
        for (int i=0; i<numArgs-1; i++ ){
          myargs[i] = strdup(cmdArgs[i]);
        }

        myargs[numArgs-1] = NULL;
        execvp(myargs[0], myargs); 
      }
      printf("Command Not Found\n");
      return 0; 
  } 
  else {

      currBgProcess->pid = rc;
      backgroundProcesses[numBackgroundProcesses-1] = *currBgProcess;
      free(currBgProcess);

      currprocPid = rc;

      
      if (strcmp(cmdArgs[numArgs-1], "&") != 0){
        int wc = waitpid(rc, NULL, WUNTRACED); // Wait for the child to terminate or stop

        if (wc > 0){
          currprocPid = 0; 
         
        }
        if (strcmp(backgroundProcesses[numBackgroundProcesses-1].status, "FG") == 0){
          removeProcessFromList(rc); 
        }
        signal(SIGCHLD,child_handler);
      }
      else {
        signal(SIGCHLD,child_handler);
        
      }
  }
  return 0;
}

int getcmd(char **buf, size_t *n) {
    printf("wsh> ");

    shellPid = getpid(); 
    ssize_t read = getline(buf, n, stdin);

    if (read != -1) {
        (*buf)[read-1] = '\0';
    }
    // parsing the buf into multiple pieces
    char *token;
    char *delimiter = " ";
    char *temp = *buf;


    // getting all the args
    cmdArgs = malloc(sizeof(char*));
    numArgs = 0;

    while ((token = strsep(&temp, delimiter)) != NULL) {
      // Allocate memory for each argument and copy the token
      cmdArgs[numArgs] = malloc(strlen(token) + 1);
      strcpy(cmdArgs[numArgs], token);
      if (strcmp(token, strdup("|")) == 0){
        isPipe = 1; 
      }
      numArgs++;
      // Reallocate memory for the pointer array
      cmdArgs = realloc(cmdArgs, (numArgs + 1) * sizeof(char*));
    }


    if (strcmp(*buf, "exit") == 0) {
        free(*buf); 
        exit(0); 
    } else if (strcmp(cmdArgs[0], "cd") == 0) {
        if (numArgs != 2){
          printf("No Directory Specified\n");
        }
        else {

          chdir(cmdArgs[1]);
        }
    } 
    else if (strcmp(*buf, "jobs") == 0) {
      printJobs(); 

    } 
    else if (strcmp(cmdArgs[0], "fg") == 0) {
       // printf("we have run itnto fg\n"); 
        if (numArgs == 1){
        // find largest stopped job
        find_running_job(0); 

        }
        else if (numArgs == 2){
          find_running_job(atoi(cmdArgs[1]));
        }
    } 
    else if (strcmp(cmdArgs[0], "bg") == 0) {
        //printf("You entered 'bg'.\n");
        // need to get number of args
        if (numArgs == 1){
          // find largest stopped job
          find_stopped_job(0); 

        }
        else if (numArgs == 2){
          find_stopped_job(atoi(cmdArgs[1]));
        }
    }
    else if (isPipe){
      execPipe(temp); 
    }

    else {
      execCmd(); 
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    char *buf = NULL;
    size_t n = 0;

    signal(SIGTSTP, sigtstp_handler);
    signal(SIGINT, sigint_handler);


    // create an infinite loop to print output
    while(1) {
        getcmd(&buf, &n);
    }

    free(buf);
    return 0;
}
