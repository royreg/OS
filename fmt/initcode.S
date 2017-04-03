8900 # Initial process execs /init.
8901 # This code runs in user space.
8902 
8903 #include "syscall.h"
8904 #include "traps.h"
8905 
8906 
8907 # exec(init, argv)
8908 .globl start
8909 start:
8910   pushl $argv
8911   pushl $init
8912   pushl $0  // where caller pc would be
8913   movl $SYS_exec, %eax
8914   int $T_SYSCALL
8915 
8916 # for(;;) exit();
8917 exit:
8918   movl $SYS_exit, %eax
8919   int $T_SYSCALL
8920   jmp exit
8921 
8922 # char init[] = "/bin/init\0";
8923 init:
8924   .string "/bin/init\0"
8925 
8926 # char *argv[] = { init, 0 };
8927 .p2align 2
8928 argv:
8929   .long init
8930   .long 0
8931 
8932 
8933 
8934 
8935 
8936 
8937 
8938 
8939 
8940 
8941 
8942 
8943 
8944 
8945 
8946 
8947 
8948 
8949 
