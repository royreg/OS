3650 #include "types.h"
3651 #include "defs.h"
3652 #include "param.h"
3653 #include "memlayout.h"
3654 #include "mmu.h"
3655 #include "proc.h"
3656 #include "x86.h"
3657 #include "syscall.h"
3658 
3659 // User code makes a system call with INT T_SYSCALL.
3660 // System call number in %eax.
3661 // Arguments on the stack, from the user call to the C
3662 // library system call function. The saved user %esp points
3663 // to a saved program counter, and then the first argument.
3664 
3665 // Fetch the int at addr from the current process.
3666 int
3667 fetchint(uint addr, int *ip)
3668 {
3669   if(addr >= proc->sz || addr+4 > proc->sz)
3670     return -1;
3671   *ip = *(int*)(addr);
3672   return 0;
3673 }
3674 
3675 // Fetch the nul-terminated string at addr from the current process.
3676 // Doesn't actually copy the string - just sets *pp to point at it.
3677 // Returns length of string, not including nul.
3678 int
3679 fetchstr(uint addr, char **pp)
3680 {
3681   char *s, *ep;
3682 
3683   if(addr >= proc->sz)
3684     return -1;
3685   *pp = (char*)addr;
3686   ep = (char*)proc->sz;
3687   for(s = *pp; s < ep; s++)
3688     if(*s == 0)
3689       return s - *pp;
3690   return -1;
3691 }
3692 
3693 // Fetch the nth 32-bit system call argument.
3694 int
3695 argint(int n, int *ip)
3696 {
3697   return fetchint(proc->tf->esp + 4 + 4*n, ip);
3698 }
3699 
3700 // Fetch the nth word-sized system call argument as a pointer
3701 // to a block of memory of size bytes.  Check that the pointer
3702 // lies within the process address space.
3703 int
3704 argptr(int n, char **pp, int size)
3705 {
3706   int i;
3707 
3708   if(argint(n, &i) < 0)
3709     return -1;
3710   if(size < 0 || (uint)i >= proc->sz || (uint)i+size > proc->sz)
3711     return -1;
3712   *pp = (char*)i;
3713   return 0;
3714 }
3715 
3716 // Fetch the nth word-sized system call argument as a string pointer.
3717 // Check that the pointer is valid and the string is nul-terminated.
3718 // (There is no shared writable memory, so the string can't change
3719 // between this check and being used by the kernel.)
3720 int
3721 argstr(int n, char **pp)
3722 {
3723   int addr;
3724   if(argint(n, &addr) < 0)
3725     return -1;
3726   return fetchstr(addr, pp);
3727 }
3728 
3729 extern int sys_chdir(void);
3730 extern int sys_close(void);
3731 extern int sys_dup(void);
3732 extern int sys_exec(void);
3733 extern int sys_exit(void);
3734 extern int sys_fork(void);
3735 extern int sys_fstat(void);
3736 extern int sys_getpid(void);
3737 extern int sys_kill(void);
3738 extern int sys_link(void);
3739 extern int sys_mkdir(void);
3740 extern int sys_mknod(void);
3741 extern int sys_open(void);
3742 extern int sys_pipe(void);
3743 extern int sys_read(void);
3744 extern int sys_sbrk(void);
3745 extern int sys_sleep(void);
3746 extern int sys_unlink(void);
3747 extern int sys_wait(void);
3748 extern int sys_write(void);
3749 extern int sys_uptime(void);
3750 extern int sys_priority(void);
3751 extern int sys_policy(void);
3752 
3753 static int (*syscalls[])(void) = {
3754 [SYS_fork]    sys_fork,
3755 [SYS_exit]    sys_exit,
3756 [SYS_wait]    sys_wait,
3757 [SYS_pipe]    sys_pipe,
3758 [SYS_read]    sys_read,
3759 [SYS_kill]    sys_kill,
3760 [SYS_exec]    sys_exec,
3761 [SYS_fstat]   sys_fstat,
3762 [SYS_chdir]   sys_chdir,
3763 [SYS_dup]     sys_dup,
3764 [SYS_getpid]  sys_getpid,
3765 [SYS_sbrk]    sys_sbrk,
3766 [SYS_sleep]   sys_sleep,
3767 [SYS_uptime]  sys_uptime,
3768 [SYS_open]    sys_open,
3769 [SYS_write]   sys_write,
3770 [SYS_mknod]   sys_mknod,
3771 [SYS_unlink]  sys_unlink,
3772 [SYS_link]    sys_link,
3773 [SYS_mkdir]   sys_mkdir,
3774 [SYS_close]   sys_close,
3775 [SYS_priority] sys_priority,
3776 [SYS_priority] sys_policy
3777 };
3778 
3779 void
3780 syscall(void)
3781 {
3782   int num;
3783 
3784   num = proc->tf->eax;
3785   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
3786     proc->tf->eax = syscalls[num]();
3787   } else {
3788     cprintf("%d %s: unknown sys call %d\n",
3789             proc->pid, proc->name, num);
3790     proc->tf->eax = -1;
3791   }
3792 }
3793 
3794 
3795 
3796 
3797 
3798 
3799 
