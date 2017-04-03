3800 #include "types.h"
3801 #include "x86.h"
3802 #include "defs.h"
3803 #include "date.h"
3804 #include "param.h"
3805 #include "memlayout.h"
3806 #include "mmu.h"
3807 #include "proc.h"
3808 
3809 int
3810 sys_fork(void)
3811 {
3812   return fork();
3813 }
3814 
3815 int
3816 sys_exit(void)
3817 {
3818   int status;
3819   if(argint(0,&status)<0)
3820     return -1;
3821 
3822   exit(status);
3823   return 0;  // not reached
3824 }
3825 int
3826 sys_priority(void)
3827 {
3828   int p;
3829   if(argint(0,&p)<0)
3830     return -1;
3831 
3832   int pri = priority(p);
3833   return pri;  // not reached
3834 }
3835 
3836 int
3837 sys_policy(void)
3838 {
3839   int p;
3840   if(argint(0,&p)<0)
3841     return -1;
3842 
3843   int pol = policy(p);
3844   return pol;  // not reached
3845 }
3846 
3847 
3848 
3849 
3850 int
3851 sys_wait(void)
3852 {
3853   int* status;
3854   int retVal;
3855   if(argptr(0,(char**)&status,sizeof(status))>=0)
3856     retVal=wait(status);
3857   else
3858     retVal=-1;
3859 
3860   return retVal;
3861 }
3862 
3863 int
3864 sys_kill(void)
3865 {
3866   int pid;
3867 
3868   if(argint(0, &pid) < 0)
3869     return -1;
3870   return kill(pid);
3871 }
3872 
3873 int
3874 sys_getpid(void)
3875 {
3876   return proc->pid;
3877 }
3878 
3879 int
3880 sys_sbrk(void)
3881 {
3882   int addr;
3883   int n;
3884 
3885   if(argint(0, &n) < 0)
3886     return -1;
3887   addr = proc->sz;
3888   if(growproc(n) < 0)
3889     return -1;
3890   return addr;
3891 }
3892 
3893 
3894 
3895 
3896 
3897 
3898 
3899 
3900 int
3901 sys_sleep(void)
3902 {
3903   int n;
3904   uint ticks0;
3905 
3906   if(argint(0, &n) < 0)
3907     return -1;
3908   acquire(&tickslock);
3909   ticks0 = ticks;
3910   while(ticks - ticks0 < n){
3911     if(proc->killed){
3912       release(&tickslock);
3913       return -1;
3914     }
3915     sleep(&ticks, &tickslock);
3916   }
3917   release(&tickslock);
3918   return 0;
3919 }
3920 
3921 // return how many clock tick interrupts have occurred
3922 // since start.
3923 int
3924 sys_uptime(void)
3925 {
3926   uint xticks;
3927 
3928   acquire(&tickslock);
3929   xticks = ticks;
3930   release(&tickslock);
3931   return xticks;
3932 }
3933 
3934 
3935 
3936 
3937 
3938 
3939 
3940 
3941 
3942 
3943 
3944 
3945 
3946 
3947 
3948 
3949 
