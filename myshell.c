#include "myshell_parser.h"
#include "stddef.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void shell_prompt(){
    printf("my_shell$");
}

char *shell_read(){
    char *command=NULL;    
    size_t line=0;
    if(getline(&command,&line,stdin)==-1){
        //perror("readcommand");
        free(command);
        exit(EXIT_SUCCESS);
    }else{
        //exit(EXIT_SUCCESS);
    }
    return command; 
}

void exec_basic(struct pipeline *pp1){
    pid_t pid;
    int status;
    pid=fork();
    if(pid==-1){
        perror("ERROR");
        exit(EXIT_FAILURE);        
    }
    if(pid==0){
        /*child processes*/
        if(execvp(pp1->commands->command_args[0],pp1->commands->command_args)==-1){
            perror("ERROR");
            exit(EXIT_FAILURE);
        }                
    }
    if(pid>0){
        /*parent processes*/
        if(!pp1->is_background){
            waitpid(pid,&status,0);
            
        }else{
            while(waitpid(-1,&status,WNOHANG)>0){                
            }
            printf("\n");
        }                          
    }    
}

void exec_file(struct pipeline *pp1){
    int status;
    pid_t pid;
    pid=fork();
    if(pid==-1){
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    if(pid==0){
        /*child processes*/
        int filein;
        int fileout;
        if(pp1->commands->redirect_in_path){
            if((filein=open(pp1->commands->redirect_in_path,O_RDONLY, 0600))<0){
                perror("ERROR");
                exit(EXIT_FAILURE);
            }
            dup2(filein,STDIN_FILENO);
            close(filein);
        }
        if(pp1->commands->redirect_out_path){
            if((fileout=open(pp1->commands->redirect_out_path,O_CREAT | O_TRUNC | O_WRONLY, 0600))<0){
                perror("ERROR");
                exit(EXIT_FAILURE);
            }
            dup2(fileout,STDOUT_FILENO);
            close(fileout);
        }
        if(execvp(pp1->commands->command_args[0],pp1->commands->command_args)==-1){
            perror("ERROR");
            exit(EXIT_FAILURE);
        }
    }
    if(pid>0){
        /*parent processes*/
        if(!pp1->is_background){
            waitpid(pid,&status,0);
        }else{
            while(waitpid(-1,&status,WNOHANG)>0){                
            }
            printf("\n");
        }
    }
    
}

void exec_pipe(struct pipeline *pp1){
    struct pipeline_command *p1=pp1->commands;
    int status;
    pid_t pid;
    int numpipes=0;
    while(p1->next){
        numpipes++;
        p1=p1->next;
    }
    p1=pp1->commands;
    int pipefds[2*numpipes];
    /*create all pipes*/
    for(int i=0;i<numpipes;i++){
        if(pipe(pipefds+2*i)<0){
            perror("ERROR");
            exit(EXIT_FAILURE);
        }        
    }
    int i=0;
    while(p1){
        pid=fork();
        if(pid==-1){
            perror("ERROR");
            exit(EXIT_FAILURE);
        }
        if(pid==0){
            /*child processes*/
            /*check for redirected file*/
            if(p1->redirect_in_path){
                int fileds;
                if((fileds=open(p1->redirect_in_path,O_RDONLY, 0600))<0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
                if(dup2(fileds,STDIN_FILENO)<0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
                close(fileds);
            }
            if(p1->redirect_out_path){
                int fileds;
                if((fileds=open(p1->redirect_out_path,O_CREAT | O_TRUNC | O_WRONLY, 0600))<0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
                if(dup2(fileds,STDOUT_FILENO)<0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
                close(fileds);
            }
            /*if have previous command*/
            if(i!=0){
                if(dup2(pipefds[i-2],STDIN_FILENO)<0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
            }
            /*if have next command*/
            if(p1->next){
                if(dup2(pipefds[i+1],STDOUT_FILENO)<0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
            }
            /*close pipes*/
            for(int j=0;j<2*numpipes;j++){
                close(pipefds[j]);
            }
            /*exec*/
            if(execvp(p1->command_args[0],p1->command_args)==-1){
                perror("ERROR");
                exit(EXIT_FAILURE);
            }
        }
        p1=p1->next;
        i+=2;
    }
        
        /*parent processes*/
    for(int j=0;j<2*numpipes;j++){
        close(pipefds[j]);
    }
    if(!pp1->is_background){
        while(waitpid(-1,&status,0)>0){
        }
        //printf("\n");
    }else{
        while(waitpid(-1,&status,WNOHANG)>0){                
        }
        printf("\n");
    }
}

int main(int argc, char **argv){
    //char cline[MAX_LINE_LENGTH];
    //int status;
    do{
    char *command=NULL;
    
    //promptif
    if(argc==2){
        if(strcmp(argv[1],"-n")!=0)
        {
            shell_prompt(); 
        }
    }else{
        shell_prompt(); 
    }
    
        
    //read line
    command=shell_read();
    
    //parser line
    struct pipeline *pp1=pipeline_build(command);
    
    if(pp1==NULL){
        //fprintf(stderr, "NOINPUTFILE");
        //exit(-1);
        errno=5;
        perror("ERROR");
        if(errno!=0){
            errno=0;
            continue;
        }
    }
        

    //fork
    if(pp1->commands->next!=NULL){
        exec_pipe(pp1);

    }else if(pp1->commands->redirect_in_path!=NULL||pp1->commands->redirect_out_path!=NULL){
        exec_file(pp1);

    }else{
        exec_basic(pp1);
    }
    free(command);
    pipeline_free(pp1);

    }while(true);
    
}