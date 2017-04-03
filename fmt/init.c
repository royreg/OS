9000 // init: The initial user-level program
9001 
9002 #include "types.h"
9003 #include "stat.h"
9004 #include "user.h"
9005 #include "fcntl.h"
9006 
9007 char *argv[] = { "sh", 0 };
9008 
9009 int
9010 main(void)
9011 {
9012   int pid, wpid;
9013 
9014   if(open("console", O_RDWR) < 0){
9015     mknod("console", 1, 1);
9016     open("console", O_RDWR);
9017   }
9018   dup(0);  // stdout
9019   dup(0);  // stderr
9020 
9021   for(;;){
9022     printf(1, "init: starting sh\n");
9023     pid = fork();
9024     if(pid < 0){
9025       printf(1, "init: fork failed\n");
9026       exit(0);
9027     }
9028     if(pid == 0){
9029       exec("/bin/sh", argv);
9030       printf(1, "init: exec sh failed\n");
9031       exit(0);
9032     }
9033     while((wpid=wait(0)) >= 0 && wpid != pid)
9034       printf(1, "zombie!\n");
9035   }
9036 }
9037 
9038 
9039 
9040 
9041 
9042 
9043 
9044 
9045 
9046 
9047 
9048 
9049 
