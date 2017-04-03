#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  

  if(argc ==2){
    int newPol=atoi(argv[1]);
    if(newPol<3&& newPol>=0){
      printf(2, "switching policy..\n");
      policy(atoi(argv[1]));
      exit(0);}
    else{
      printf(2,"unkmown policy: %d\n",newPol);
    }

  }
  else {
  	printf(2,"arguments exception%s\n"," no arguments");
  }
  return -1;
  	
}
