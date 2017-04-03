8950 #include "syscall.h"
8951 #include "traps.h"
8952 
8953 #define SYSCALL(name) \
8954   .globl name; \
8955   name: \
8956     movl $SYS_ ## name, %eax; \
8957     int $T_SYSCALL; \
8958     ret
8959 
8960 SYSCALL(fork)
8961 SYSCALL(exit)
8962 SYSCALL(wait)
8963 SYSCALL(pipe)
8964 SYSCALL(read)
8965 SYSCALL(write)
8966 SYSCALL(close)
8967 SYSCALL(kill)
8968 SYSCALL(exec)
8969 SYSCALL(open)
8970 SYSCALL(mknod)
8971 SYSCALL(unlink)
8972 SYSCALL(fstat)
8973 SYSCALL(link)
8974 SYSCALL(mkdir)
8975 SYSCALL(chdir)
8976 SYSCALL(dup)
8977 SYSCALL(getpid)
8978 SYSCALL(sbrk)
8979 SYSCALL(sleep)
8980 SYSCALL(uptime)
8981 SYSCALL(priority)
8982 SYSCALL(policy)
8983 
8984 
8985 
8986 
8987 
8988 
8989 
8990 
8991 
8992 
8993 
8994 
8995 
8996 
8997 
8998 
8999 
