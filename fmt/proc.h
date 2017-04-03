2300 // Per-CPU state
2301 struct cpu {
2302   uchar apicid;                // Local APIC ID
2303   struct context *scheduler;   // swtch() here to enter scheduler
2304   struct taskstate ts;         // Used by x86 to find stack for interrupt
2305   struct segdesc gdt[NSEGS];   // x86 global descriptor table
2306   volatile uint started;       // Has the CPU started?
2307   int ncli;                    // Depth of pushcli nesting.
2308   int intena;                  // Were interrupts enabled before pushcli?
2309   int policy;
2310 
2311   // Cpu-local storage variables; see below
2312   struct cpu *cpu;
2313   struct proc *proc;           // The currently-running process.
2314 };
2315 
2316 extern struct cpu cpus[NCPU];
2317 extern int ncpu;
2318 
2319 // Per-CPU variables, holding pointers to the
2320 // current cpu and to the current process.
2321 // The asm suffix tells gcc to use "%gs:0" to refer to cpu
2322 // and "%gs:4" to refer to proc.  seginit sets up the
2323 // %gs segment register so that %gs refers to the memory
2324 // holding those two variables in the local cpu's struct cpu.
2325 // This is similar to how thread-local variables are implemented
2326 // in thread libraries such as Linux pthreads.
2327 extern struct cpu *cpu asm("%gs:0");       // &cpus[cpunum()]
2328 extern struct proc *proc asm("%gs:4");     // cpus[cpunum()].proc
2329 
2330 
2331 // Saved registers for kernel context switches.
2332 // Don't need to save all the segment registers (%cs, etc),
2333 // because they are constant across kernel contexts.
2334 // Don't need to save %eax, %ecx, %edx, because the
2335 // x86 convention is that the caller has saved them.
2336 // Contexts are stored at the bottom of the stack they
2337 // describe; the stack pointer is the address of the context.
2338 // The layout of the context matches the layout of the stack in swtch.S
2339 // at the "Switch stacks" comment. Switch doesn't save eip explicitly,
2340 // but it is on the stack and allocproc() manipulates it.
2341 struct context {
2342   uint edi;
2343   uint esi;
2344   uint ebx;
2345   uint ebp;
2346   uint eip;
2347 };
2348 
2349 
2350 enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
2351 
2352 
2353 struct perf {
2354   int ctime;
2355   int ttime;
2356   int stime;
2357   int retime;
2358   int rutime;
2359 };
2360 
2361 
2362 
2363 // Per-process state
2364 struct proc {
2365   uint sz;                     // Size of process memory (bytes)
2366   pde_t* pgdir;                // Page table
2367   char *kstack;                // Bottom of kernel stack for this process
2368   enum procstate state;        // Process state
2369   int pid;                     // Process ID
2370   struct proc *parent;         // Parent process
2371   struct trapframe *tf;        // Trap frame for current syscall
2372   struct context *context;     // swtch() here to run process
2373   void *chan;                  // If non-zero, sleeping on chan
2374   int killed;                  // If non-zero, have been killed
2375   struct file *ofile[NOFILE];  // Open files
2376   struct inode *cwd;           // Current directory
2377   char name[16];               // Process name (debugging)
2378   int exitStat;                 //Process exit status
2379   int ntickets;                 //number of tickets for the process
2380   struct pref;
2381 
2382 
2383 
2384       //      ctime      //process creation time
2385       //      ttime        //process termination time
2386       //      stime         //the time the process spent in the SLEE PING state
2387       //      retime       //the time the process spent in the READY state
2388       //      rutime         //the time the process spent in the RUNNING state
2389 
2390 };
2391 
2392 // Process memory is laid out contiguously, low addresses first:
2393 //   text
2394 //   original data and bss
2395 //   fixed-size stack
2396 //   expandable heap
2397 
2398 
2399 
