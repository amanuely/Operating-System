
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



void get_command(void);
int cmdcounter(char rawcmd[]);
void cmdinfo(char rawcmd[]);
char ***prsecmdforpipe(char *cmdforpipe[], int numb );
char  **cmdparser( char input_command[]);
void Executecmd(char *parsedcmd[]);
void dopipe(char *arr[], int num);
char **splitcmds (char input_command[]);
void tofile(char path[]);
void cmdfromfl(char path[]) ;
int detectdirout(char cmd[]);
int detectdirin(char cmd[]);
void inputdir(char cmd[], int index );
int changeout(char file[], char cmd[]);
char  *outputdir(char cmd[], int index );




int main(int argc, char const *argv[])
{
	
  if (argc==1)
  {

   get_command();

  }

  // redirection is intended
  else if (argc==2)
  {
  // we expect redirection from the command line argument
      char argument[20];
      strcpy(argument, argv[1]);

      // get the command from the file
      if (argument[0]=='<') {
          int length= strlen(argument);
          char file[length-1];
          for (int i = 1; i < length; i++) {
              file[i-1]=argument[i];
              }

          cmdfromfl(file);
      }
      // the redirection is to write the output to the file
      else if (argument[0]=='>') {
          int length= strlen(argv[1]);
          char file[length-1];
          for (int i = 1; i < length; i++) {
              file[i-1]=argument[i];
          }
          tofile(file);
      }

  }

	return 0;
}







/* This function gets command/commands from  user until the user quits
it pass the command received to appropriate function(cmdinfo) which handles the command
*/
void get_command(void)
{
    char exit[20]="exit\n";
    char command[20];
    int compare;
    // check if the user input is end of file  character(CTRL-D)
    if (fgets(command, 20, stdin)==NULL)
     {
         compare=0;
      }
    else
    {
       compare= strcmp(command, exit);
    }
      // keep getting the command until the user types exit/ or CTLRL-D
    while (compare!=0){
      
           // printf("Getting the command\n");

            cmdinfo(command);
            dup(1);

          if (fgets(command, 20, stdin)==NULL)
          {
            compare=0;
          }
          else
          {
            compare= strcmp(command, exit);
          }
    }
}








/* 
this function returns the number of commands separated by the sign |,
*/
int cmdcounter(char rawcmd[]){
  int len= strlen(rawcmd);
  int numcmd=1;
  for (int i = 0; i < len; ++i)
  {
    if (rawcmd[i]=='|')
    {
     numcmd++;
    }
  }
  return numcmd;
}








/* this function gives us information whether
the command passed by the user is intended for piping or not
- then  passes the command to a parser function which parses the command
*/
void cmdinfo(char rawcmd[]){
  //printf("In the command info\n");
  
  int cmdnum=cmdcounter(rawcmd);
  //printf("%d\n", cmdnum);
  // no piping needed
  if (cmdnum==1)
  {

     int frmfl=detectdirin(rawcmd);
     int tofl=detectdirout(rawcmd);
     //printf("%d\n",  frmfl);
     if (frmfl>-1)
     {
      
      
    
    inputdir(rawcmd, frmfl);
      
     }
     if(tofl>-1){
      outputdir(rawcmd, tofl);

     }
     else if(frmfl==-1){


        char **parsed;
        parsed=cmdparser(rawcmd);
        Executecmd(parsed);
         

      }
    
  }
  //pipe
  else{
    char **splitcmd;
    splitcmd=splitcmds(rawcmd);
    dopipe(splitcmd,cmdnum);

  }

}





/*
cmdparser function  takes in a string of a command
parses a command int an array which is suitable to be   exec function parametre
*/

char  **cmdparser( char input_command[]){
    static char  *array[11];
    int num_commands=0;
    const char delims[]=" ";
    int i=0;
    char *parsed=strtok(input_command,delims);
    while (parsed!=NULL) {
      num_commands++;
      int len=strlen(parsed);
      if (parsed[len-1]=='\n') {
        parsed[len-1]='\0';
      }
      array[i++]=parsed;
      parsed=strtok(NULL,delims);
    }
  array[num_commands]=NULL;
  return array;
}





/*
This function splits commands intended for piping into an individual commands
returns the splitted commands as an array of commands
*/
char **splitcmds (char input_command[])
{
  int num_commands=0;
  int i=0;
  char *cmd;
  static char *cmd_array[11];
  const char delims[]="|";
  cmd=strtok(input_command,delims);
  while (cmd!=NULL) {
      num_commands++;
      int len=strlen(cmd);
      if (cmd[len-1]=='\n')
      {
           cmd[len-1]='\0';
      }
      cmd_array[i++]=cmd;
      cmd=strtok(NULL,delims);
  }
return cmd_array;
}


/* this function gets command from a given file and
pass to the parser and executor function
This function is used for the redirection purpose

*/

void cmdfromfl(char path[]) {
  char buffer[128];
  //int fd= open(path,O_RDWR);
  FILE *myfile;
  myfile=fopen(path ,"r");
  
  while (!feof(myfile)) {
    char line[100];
   if (fgets(line,100,myfile)!=NULL) {
    //printf(" line from the line is %s\n", line);
    //printf(" the line %s\n", line);
    int linelen= strlen(line);
    for (int i = 0; i < linelen; ++i)
    {
      if (line[i]=='\n')
      {
        line[i]='\0';
      }
    }
      cmdinfo(line);
   }
    
  }
  //fclose(myfile);
}





/*
this function changes the standard output of a given command  to a file passed
*/
void tofile(char path[]) {
  int fd= open(path,O_RDWR);
  if (fd==-1) {
    perror("Error");
    exit(0);
  }
  dup2(fd,1);
  close(1);
  get_command();
}






/*
this function does the execution of  a given command
*/
void Executecmd(char *parsedcmd[])

{
 
  
  pid_t pid;
  pid=fork();
  if (pid==-1)
  {
    perror("Error with forking ");
    exit(0);
  }
  

  else
  {     // child process
      if (pid==0)
      {
          if (execvp(parsedcmd[0], parsedcmd)==-1)
           {

            perror("Exec failed ");

            exit(0);
            }
      }
      // parent process
      else
      {
        wait(&pid);
      }
  }

}





/*
This function is used for piping
given an array of commands,
*/

void dopipe(char *cmdArray[], int numcmds)
{

    int fd[2*numcmds];

    // set up the pipe, for each command 
    for (int i = 0; i < 2*numcmds; i+=2)
    {
    pipe(fd+i);
    }

    // we fork the process for each of the command in the cmdarray
    for (int j= 0; j < numcmds; ++j)
    {
  
      int pid;
      pid=fork();

      /* In the child process,
      for non first command, get the stdin from the read end pipe of the previous command
       for non last command, we write the standard output of the
       current process to the corresponding write end of the pipe

      */
        if (pid==-1)
        {
          perror("Error in forking");
          exit(0);
        }
        else if (pid==0)
        {

            if (j!=0)
            {
                if ( dup2(fd[2*j-2],0)<0)
                {
                  perror("error with dup");
                  exit(0);
                }
            }
            if (j+1<numcmds){
               if ( dup2(fd[2*j+1],1)<0)
               {
                    perror("Error with dup");
                    exit(0);
               }
            }
              char **parsed;
              parsed=cmdparser(cmdArray[j]);
              Executecmd(parsed);
              exit(1);
        }
        else{
          
          close(fd[2*j+1]);
          if (j>0)
          {
            close(fd[2*j-2]);
          }
        }
    }
}



int detectdirin(char cmd[]){
  int present=-1;
  int len=strlen(cmd);
  for (int i = 0; i < len; ++i)
  {
    if (cmd[i]=='<')
    {
      present=i;
    }
  }
  return present;
}




int detectdirout(char cmd[]){
  int present=-1;
  int len=strlen(cmd);
  for (int i = 0; i < len; ++i)
  {
    if (cmd[i]=='>')
    {
      present=i;
    }
  }
  return present;
}




 void inputdir(char cmd[], int index ){
   static char  path[50];
  int length= strlen(cmd);
  for (int i = index+1; i < length; ++i)
  {
    
     path[i-1]=cmd[i]; 
    
    
  }
  int len= strlen(path);
  if (path[len-1]=='\n') {
        path[len-1]='\0';
      }
  
 // printf("%s the file to read from\n",path );
  cmdfromfl(path);
 
}





char  *outputdir(char cmd[], int index ){
  char path[50];
  char nwcmd[40];
  int length= strlen(cmd);
  for (int z = 0; z < index; ++z)
  {
    nwcmd[z]=cmd[z];
  }
  for (int i = index+1; i < length; ++i)
  {
    path[i-index-1]=cmd[i]; 
  }

int lencm= strlen(nwcmd);
  for (int i = 0; i < lencm; ++i)
  {
    if (nwcmd[i]=='\n') {
        nwcmd[i]='\0';
      }
     // printf("%c\n", path[i]);
  }



int len= strlen(path);
  for (int i = 0; i < len; ++i)
  {
    if (path[i]=='\n') {
        path[i]='\0';
      }
      
  }
  int len2= strlen(path);
 
  changeout(path,nwcmd);
  return cmd;
}






int changeout(char file[], char cmd[]){
  int fd= open(file,O_RDWR|O_APPEND|O_CREAT,S_IRUSR);
  if (fd==-1) {
    perror("Error with the file");
    exit(0);
  }
  
  int outpid;
  outpid=fork();
  if (outpid==0)
  {
    dup2(fd,1);

  cmdinfo(cmd);

  exit(1);
  }
  else{
    wait(&outpid);


  }
  

 return 0;
   
}



