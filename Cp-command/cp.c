
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
int fcopy( char *, char*);


int main(int argc, char const *argv[]) {
    char source[20], destination[20];
     if (argc<3) {
       printf(" you need to supply two arguments:Source file  and destination file \n");
       exit(0);
     }
     else{
       strncpy(source, argv[1],20);
       strncpy(destination, argv[2], 20);
       fcopy(source, destination);

     }
}



/* this is a function which does the copying  */
int fcopy( char  *source, char *destination) {
  struct stat s;
  struct stat d;
  int sfd;
  int dfd;
  int dirfd;
  int num_read=0;
  int num_written=0;
  int s_info;
  int d_info;
  ssize_t  buffsize=1000;
  char buffer[buffsize];


  sfd=open(source,O_RDONLY);
  s_info=fstat(sfd, &s);
  d_info=stat(destination, &d);


  if (sfd==-1) {
      perror("Error with the source file");
      exit(1);
    }

  // when the source file exists and it is a regular file i.e readable
  if (s_info==0&& s.st_mode &S_IFREG)
  {
        if (d_info==0)
        {

            /*
            Destination file is a directory
            update the destination file to be inside the directory
            create a new file inside the directory
            copy the source file to the newly created file inside the directory
            */
            if (d.st_mode &S_IFDIR)
                {
                  close(dfd);
                  char slash[20];
                  char newpath[120];
                  strcat(newpath, destination);
                  strncpy(slash, "/",20);
                  strcat(newpath, slash);
                  strcat(newpath,source);

                 dirfd=open(newpath,O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU);
                 do
                 {
                      num_read=read(sfd, buffer, buffsize);
                      num_written= write(dirfd, buffer,num_read);
                  }while(num_read>0);
                 close(sfd);
                 close(dirfd);
                }

                /*
                Destination file exists and it is a regular file
                so we overwrite it and write to it
                */
            else if(s.st_mode &S_IFREG)
            {
              dfd=open(destination,O_WRONLY|O_TRUNC);
              do{
                  num_read=read(sfd, buffer, buffsize);
                  num_written= write(dfd, buffer,num_read);
                }while(num_read>0);
              close(dfd);
              close(sfd);
            }
        }


        /* Destination file does not exist
        create a new file and write to it
        */
        else
          {
            dfd=open(destination,O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU);
             do{
                  num_read=read(sfd, buffer, buffsize);
                  num_written= write(dfd, buffer,num_read);
                }while(num_read>0);
              close(dfd);
              close(sfd);
          }
  }
  else
     {
         printf("the source file is  not a regular file\n");
         exit(1);
    }

}
