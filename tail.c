#include "types.h"
#include "stat.h"
#include "user.h"
#include <stdbool.h>

//define nullpointer
#define NULL ((void *) 0)

//create receiving buffer
char buffer[512];

//create node for a output buffer linked list
typedef struct node{
	int line_n;
	char output_buffer[512];
	struct node *prev;
	struct node *next;
}output_buffer_node;

//tail function
//input parameters: file descriptor, line_number, file name
void tail(int fd, int line_number, char *file_name){
	int n,counter;
	//create the head pointer of the output buffer
	output_buffer_node *head = NULL;
	//tail pointer of the list
	output_buffer_node *tail = NULL;
	//temporary pointers
	output_buffer_node *temp = NULL;
	output_buffer_node *to_delete = NULL;
	//create a output buffer linked list
	head = malloc(sizeof(output_buffer_node));
	head->prev = NULL;
	head->next = NULL;
	head->line_n = 0;
	tail = head;
	//read in characters from the file until the end of file
	while((n=read(fd,buffer,sizeof(buffer))) > 0){
		int i;
		for(i=0;i<n;i++){
			//save characters from file to buffer
			tail->output_buffer[i] = buffer[i];
			//check if the character is new line or carriage return
			//if yes then save number of lines in each node
			if(buffer[i]=='\n' || buffer[i]=='\r'){
				tail->line_n = tail->line_n + 1;
			}
		}
		//save current node address
		temp = tail;
		//create a new node for the output buffer list
		tail->next = malloc(sizeof(output_buffer_node));
		//point to the new node
		tail = tail->next;
		//previous node
		tail->prev = temp;
		tail->next = NULL;
		//reset the line_n counter (counts the number of lines in each node)
		tail->line_n = 0;
	}
	//iterator of the list
	output_buffer_node *iter = tail;
	//go back to fetch lines
	//count the total number of lines stored in the nodes as we go back
	//stop when exceede target (target line can be stored in mulitple nodes)
	for(counter=iter->line_n;counter<line_number;){
		//stop if iter has reached head
		if(iter->prev!=NULL){
			iter = iter->prev;
			counter += iter->line_n;
		}
		else{
			break;
		}
	}
	//delete all other nodes from the beginning of the list up to iter
	//we don't need those. first check if iter is already at head
	if(iter->prev!=NULL){
		temp = iter->prev;
		while(temp->prev != NULL){
			//set to_delete to current node
			to_delete = temp;
			//point to previous node
			temp = temp->prev;
			//delete current node
			free(to_delete);
		}
	}
	//print the content of the output buffer and delete each node after
	//printing
	while(iter != NULL){
		int k = sizeof(iter->output_buffer);
		int i;
		for(i=0;i<k;i++){
			//ignore all the characters until the desired line is reached
			if(counter>line_number){
				if(iter->output_buffer[i]=='\n'||iter->output_buffer[i]=='\r')
					counter = counter - 1;
			}
			else{
				//not printing NUL characters
				if(iter->output_buffer[i]!='\0')
					printf(1,"%c",iter->output_buffer[i]);
			}
		}
		//set to_delete to current node after printing its content
		to_delete = iter;
		//point to the next node
		iter = iter->next;
		iter->prev = NULL;
		//delete current node
		free(to_delete);
	}
}

//test function: check if the first character of the input argument is a '-'
bool test(char input[]){
	if(input[0]=='-')
		return 1;
	return 0;
}

//convert function: converts input string argument to integer
int convert(char input[]){
	//input[0] is '-', actual argument starts after it
	//atoi() only accepts char * so use &input[1]
	return (int)atoi(&input[1]);
}

//main function
int main(int argc, char *argv[]){
	int fd,i;

	//check if no argument has been provided
	if(argc<=1){
		//call head with fd=0. head will read from pipe or standard
		//input
		tail(0,10,"");
		exit();
	}
	//check if only 1 argument has been provided
	else if(argc==2){
		//test if argument is a file name. if yes default to 10
		//lines
		if(!test(argv[1])){
			tail(fd=open(argv[1],0),10,argv[1]);
		}
		//test if only line number has been specified
		else{
			tail(0,convert(argv[1]),"");
		}
		close(fd);
		exit();
	}
	//if more than 2 arguments i.e. more than 1 file, have been provided
	else{
		for(i=2;i<argc;i++){
			//if the files cannot be opened print error message
			if((fd=open(argv[i],0))<0){
				printf(1,"tail: cannot open %s\n",argv[i]);
				exit();
			}
			//call the tail function
			tail(fd,convert(argv[1]),argv[i]);
			close(fd);
		}
		exit();
	}
}
