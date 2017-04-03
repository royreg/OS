#include "types.h"
#include "stat.h"
#include "user.h"


void fib(int n) {
	if (n <= 1)
		return;
	fib(n-1);
	fib(n-2);
}


int
main(int argc, char **argv)
{
	int i;
  for(i=0; i<30;i++){
  	int pid = fork();

  	if(pid==0){
  		fib(39);
  	}
  	else{
  		continue;
  	}
  } 
}
