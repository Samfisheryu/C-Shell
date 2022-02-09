#include "myshell_parser.h"
#include "stddef.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct pipeline *pipeline_build(const char *command_line)
{
	// TODO: Implement this function
	char cl[MAX_ARGV_LENGTH]="";
    char cl2[MAX_ARGV_LENGTH]="";
	strcpy(cl,command_line);
    strcpy(cl2,command_line);
	//=command_line;
	//create a pipeline structure;
    struct pipeline *result=(struct pipeline*)malloc(sizeof(struct pipeline));
	struct pipeline_command *p1,*head;
	p1=(struct pipeline_command*)malloc(sizeof(struct pipeline_command));
	head=p1;
	p1->redirect_in_path=NULL;
    p1->redirect_out_path=NULL;
	result->commands=NULL;
	
	
	//check if it runs at background
	char *findback=strchr(cl2,'&');
	result->is_background=(findback!=NULL)?true:false;
    if(findback!=NULL){
    findback=strchr(findback+1,'&');
    if(findback!=NULL) return NULL;
    char *checkb=strtok(cl2,"&");
    checkb=strtok(NULL," \n\t");
    if(checkb!=NULL) return NULL;
    }

	//parse the command
    int commandindex=0;
    bool islast=false;
	char *bar="|";
    char *hasnext=strchr(cl,'|');
	char *token=strtok(cl,bar);
	if(token==NULL){
		result->commands=NULL;
		return result;
	}
    bool hasr=false;
	while(token!=NULL){
		char *original=strtok(NULL,"");
		char temp1[MAX_ARGV_LENGTH]="";
		if(strchr(token,'>')!=NULL&&strchr(token,'<')!=NULL){
            hasr=true;
            if(commandindex!=0||hasnext!=NULL){
                free(result);
                free(p1);
                return NULL;
            }
			char symbol1=(strchr(token,'>')<strchr(token,'<'))?'>':'<';
			char symbol2=(strchr(token,'>')<strchr(token,'<'))?'<':'>';
			char temp2[MAX_ARGV_LENGTH]="";
			char temp3[MAX_ARGV_LENGTH]="";
			int size1=strchr(token,symbol1)-token;
			strncpy(temp1,token,size1);
			int size2=strchr(token,symbol2)-strchr(token,symbol1)-1;
			strncpy(temp2,strchr(token,symbol1)+1,size2);
			int size3=strlen(token)-(strchr(token,symbol2)-token+1);
			strncpy(temp3,strchr(token,symbol2)+1,size3);
			char *file1=strtok(temp2," \n\t&");
            if(file1==NULL){
                free(result);
                free(p1);
                return NULL;
            }
			if(symbol1=='<'){
                if(strchr(file1,'<')!=NULL){
                    free(result);
                    free(p1);
                    return NULL;
                }
				p1->redirect_in_path=(char *)malloc(sizeof(char)*MAX_ARGV_LENGTH);
				strcpy(p1->redirect_in_path,file1);
			}else{
                if(strchr(file1,'>')!=NULL){
                    free(result);
                    free(p1);
                    return NULL;
                }
				p1->redirect_out_path=(char *)malloc(sizeof(char)*MAX_ARGV_LENGTH);
				strcpy(p1->redirect_out_path,file1);
			}
            file1=strtok(NULL," \n\t&");
            if(file1!=NULL){
                free(result);
                free(p1);
                return NULL;
            }
			char *file2=strtok(temp3," \n\t&");
            if(file2==NULL){
                free(result);
                free(p1);
                return NULL;
            }
			if(symbol1=='>'){
                if(strchr(file2,'<')!=NULL){
                    free(result);
                    free(p1);
                    return NULL;
                }
				p1->redirect_in_path=(char *)malloc(sizeof(char)*MAX_ARGV_LENGTH);
				strcpy(p1->redirect_in_path,file2);
			}else{
                if(strchr(file2,'>')!=NULL){
                    free(result);
                    free(p1);
                    return NULL;
                }
				p1->redirect_out_path=(char *)malloc(sizeof(char)*MAX_ARGV_LENGTH);
				strcpy(p1->redirect_out_path,file2);
			}
            file2=strtok(NULL," \n\t&");
            if(file2!=NULL){
                free(result);
                free(p1);
                return NULL;
            }
		}else if(strchr(token,'>')!=NULL||strchr(token,'<')!=NULL){
            hasr=true;
			char symbol=(strchr(token,'>')!=NULL)?'>':'<';
			char temp2[MAX_ARGV_LENGTH]="";
			int size1=strchr(token,symbol)-token;
			strncpy(temp1,token,size1);
			int size2=strlen(token)-(strchr(token,symbol)-token+1);
			strncpy(temp2,strchr(token,symbol)+1,size2);
			char *file=strtok(temp2," \n\t&");
            if(file==NULL){
                free(result);
                free(p1);
                return NULL;
            }
			if(symbol=='<'){
                if(strchr(file,'<')!=NULL){
                    free(result);
                    free(p1);
                    return NULL;
                }
                if(commandindex!=0) return NULL;
				p1->redirect_in_path=(char *)malloc(sizeof(char)*MAX_ARGV_LENGTH);
				strcpy(p1->redirect_in_path,file);
			}else{
                if(strchr(file,'>')!=NULL){
                    free(result);
                    free(p1);
                    return NULL;
                }
                islast=true;
				p1->redirect_out_path=(char *)malloc(sizeof(char)*MAX_ARGV_LENGTH);
				strcpy(p1->redirect_out_path,file);
			}
            file=strtok(NULL," \n\t&");
            if(file!=NULL){
                free(result);
                free(p1);
                return NULL;
            }
		}else{
			strcpy(temp1,token);
		}
		char *word=strtok(temp1," &\n\t");
		int index=0;
        if(word==NULL&&hasr){
            free(result);
            free(p1);
            return NULL;
        }
		while(word!=NULL){
            hasr=false;
			p1->command_args[index]=(char *)malloc(sizeof(char)*MAX_ARGV_LENGTH);
			strcpy(p1->command_args[index],word);
			index++;
			word=strtok(NULL," &\n\t");
			p1->command_args[index]=NULL;
		}
        //char *hasnext=strchr(original,'|');
        char *tt=strtok(original,"\n\t");
        if(tt==NULL&&hasnext!=NULL){
            free(result);
            free(p1);
            return NULL;
        }
        if(original!=NULL) hasnext=strchr(original,'|');
		token=strtok(original,"|");
		if(token!=NULL){
            if(islast) return NULL;
			p1->next=(struct pipeline_command*)malloc(sizeof(struct pipeline_command*));
		    p1=p1->next;
		    p1->redirect_in_path=NULL;
		    p1->redirect_out_path=NULL;
            commandindex++;
		}else{
            p1->next=NULL;
		}
	}
	result->commands=head;

	return result;
}

void pipeline_free(struct pipeline *pipeline){
    struct pipeline_command *current,*next;
	current=pipeline->commands;
    if(current!=NULL){
        next=current->next;
    }
	while(current!=NULL){
		if(current->redirect_in_path!=NULL){
			free(current->redirect_in_path);
            current->redirect_in_path=NULL;
		}
		if(current->redirect_out_path!=NULL){
			free(current->redirect_out_path);
            current->redirect_out_path=NULL;
		}
		int index=0;
		while(current->command_args[index]!=NULL){
			free(current->command_args[index]);
            current->command_args[index]=NULL;
			index++;
		}
		free(current);
		current=next;
        if(current!=NULL){
            next=current->next;
        }
    }
	free(pipeline);
    pipeline=NULL;
}
/*
int main(){
    struct pipeline* my_pipeline = pipeline_build("ls |");
    printf("haha");
    
}*/

