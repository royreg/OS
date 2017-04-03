2400 #include "types.h"
2401 #include "defs.h"
2402 #include "param.h"
2403 #include "memlayout.h"
2404 #include "mmu.h"
2405 #include "x86.h"
2406 #include "proc.h"
2407 #include "spinlock.h"
2408 
2409 
2410 #define  UNIFORM 0
2411 #define  P_SCHED 1
2412 #define  DYNAMIC 2
2413 
2414 
2415 struct {
2416   struct spinlock lock;
2417   struct proc proc[NPROC];
2418 } ptable;
2419 
2420 static struct proc *initproc;
2421 
2422 int nextpid = 1;
2423 extern void forkret(void);
2424 extern void trapret(void);
2425 
2426 static void wakeup1(void *chan);
2427 
2428 void
2429 pinit(void)
2430 {
2431   initlock(&ptable.lock, "ptable");
2432 }
2433 
2434 
2435 
2436 
2437 
2438 
2439 
2440 
2441 
2442 
2443 
2444 
2445 
2446 
2447 
2448 
2449 
2450 // Look in the process table for an UNUSED proc.
2451 // If found, change state to EMBRYO and initialize
2452 // state required to run in the kernel.
2453 // Otherwise return 0.
2454 static struct proc*
2455 allocproc(void)
2456 {
2457   struct proc *p;
2458   char *sp;
2459 
2460   acquire(&ptable.lock);
2461 
2462   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2463     if(p->state == UNUSED)
2464       goto found;
2465 
2466   release(&ptable.lock);
2467   return 0;
2468 
2469 found:
2470   p->state = EMBRYO;
2471 
2472 
2473   int poli = cpu->policy;
2474   if(poli == UNIFORM)
2475     p->ntickets = 1;
2476 
2477   if(poli == P_SCHED)
2478     p->ntickets = 10;
2479 
2480   if(poli == DYNAMIC)
2481     p->ntickets = 20;
2482 
2483 
2484   p->pid = nextpid++;
2485 
2486   release(&ptable.lock);
2487 
2488   // Allocate kernel stack.
2489   if((p->kstack = kalloc()) == 0){
2490     p->state = UNUSED;
2491     return 0;
2492   }
2493   sp = p->kstack + KSTACKSIZE;
2494 
2495   // Leave room for trap frame.
2496   sp -= sizeof *p->tf;
2497   p->tf = (struct trapframe*)sp;
2498 
2499 
2500   // Set up new context to start executing at forkret,
2501   // which returns to trapret.
2502   sp -= 4;
2503   *(uint*)sp = (uint)trapret;
2504 
2505   sp -= sizeof *p->context;
2506   p->context = (struct context*)sp;
2507   memset(p->context, 0, sizeof *p->context);
2508   p->context->eip = (uint)forkret;
2509 
2510   return p;
2511 }
2512 int priority(int pri){
2513   if(pri<=0 || cpu->policy!=P_SCHED);{
2514     return -1;
2515   }
2516 
2517   proc->ntickets=pri;
2518 
2519   return 1;
2520 }
2521 
2522 
2523 int policy(int pol){
2524   if(pol< 0 || pol>2);{
2525     return -1;
2526   }
2527   if(pol == cpu->policy)
2528     return 1;
2529   cpu->policy = pol;
2530 
2531   struct proc *p;
2532 
2533   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2534     if(pol== UNIFORM){
2535      p->ntickets = 1;
2536     }
2537     else if(pol== P_SCHED) {
2538       p->ntickets = 10;
2539     }
2540 
2541     else {
2542       p->ntickets = 20;
2543     }
2544   }
2545 
2546   return 1;
2547 
2548 }
2549 
2550 
2551 // Set up first user process.
2552 void
2553 userinit(void)
2554 {
2555   struct proc *p;
2556   extern char _binary_initcode_start[], _binary_initcode_size[];
2557 
2558   p = allocproc();
2559 
2560   initproc = p;
2561   if((p->pgdir = setupkvm()) == 0)
2562     panic("userinit: out of memory?");
2563   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2564   p->sz = PGSIZE;
2565   memset(p->tf, 0, sizeof(*p->tf));
2566   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2567   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2568   p->tf->es = p->tf->ds;
2569   p->tf->ss = p->tf->ds;
2570   p->tf->eflags = FL_IF;
2571   p->tf->esp = PGSIZE;
2572   p->tf->eip = 0;  // beginning of initcode.S
2573 
2574   safestrcpy(p->name, "initcode", sizeof(p->name));
2575   p->cwd = namei("/");
2576 
2577   // this assignment to p->state lets other cores
2578   // run this process. the acquire forces the above
2579   // writes to be visible, and the lock is also needed
2580   // because the assignment might not be atomic.
2581   acquire(&ptable.lock);
2582 
2583   p->state = RUNNABLE;
2584 
2585   release(&ptable.lock);
2586 }
2587 
2588 
2589 
2590 
2591 
2592 
2593 
2594 
2595 
2596 
2597 
2598 
2599 
2600 // Grow current process's memory by n bytes.
2601 // Return 0 on success, -1 on failure.
2602 int
2603 growproc(int n)
2604 {
2605   uint sz;
2606 
2607   sz = proc->sz;
2608   if(n > 0){
2609     if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
2610       return -1;
2611   } else if(n < 0){
2612     if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
2613       return -1;
2614   }
2615   proc->sz = sz;
2616   switchuvm(proc);
2617   return 0;
2618 }
2619 
2620 // Create a new process copying p as the parent.
2621 // Sets up stack to return as if from system call.
2622 // Caller must set state of returned proc to RUNNABLE.
2623 int
2624 fork(void)
2625 {
2626   int i, pid;
2627   struct proc *np;
2628 
2629   // Allocate process.
2630   if((np = allocproc()) == 0){
2631     return -1;
2632   }
2633 
2634   // Copy process state from p.
2635   if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
2636     kfree(np->kstack);
2637     np->kstack = 0;
2638     np->state = UNUSED;
2639     return -1;
2640   }
2641   np->sz = proc->sz;
2642   np->parent = proc;
2643   *np->tf = *proc->tf;
2644 
2645 
2646   // Clear %eax so that fork returns 0 in the child.
2647   np->tf->eax = 0;
2648 
2649 
2650   for(i = 0; i < NOFILE; i++)
2651     if(proc->ofile[i])
2652       np->ofile[i] = filedup(proc->ofile[i]);
2653   np->cwd = idup(proc->cwd);
2654 
2655   safestrcpy(np->name, proc->name, sizeof(proc->name));
2656 
2657   pid = np->pid;
2658 
2659   acquire(&ptable.lock);
2660 
2661   np->state = RUNNABLE;
2662 
2663 
2664   release(&ptable.lock);
2665 
2666   return pid;
2667 }
2668 
2669 // Exit the current process.  Does not return.
2670 // An exited process remains in the zombie state
2671 // until its parent calls wait() to find out it exited.
2672 void
2673 exit(int status)
2674 {
2675   struct proc *p;
2676   int fd;
2677 
2678   if(proc == initproc)
2679     panic("init exiting");
2680 
2681   // Close all open files.
2682   for(fd = 0; fd < NOFILE; fd++){
2683     if(proc->ofile[fd]){
2684       fileclose(proc->ofile[fd]);
2685       proc->ofile[fd] = 0;
2686     }
2687   }
2688 
2689   begin_op();
2690   iput(proc->cwd);
2691   end_op();
2692   proc->cwd = 0;
2693 
2694 
2695   acquire(&ptable.lock);
2696 
2697   // Parent might be sleeping in wait().
2698   wakeup1(proc->parent);
2699 
2700   // Pass abandoned children to init.
2701   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2702     if(p->parent == proc){
2703       p->parent = initproc;
2704       if(p->state == ZOMBIE)
2705         wakeup1(initproc);
2706     }
2707   }
2708    proc->exitStat=status;
2709   // Jump into the scheduler, never to return.
2710   proc->state = ZOMBIE;
2711   sched();
2712   panic("zombie exit");
2713 }
2714 
2715 // Wait for a child process to exit and return its pid.
2716 // Return -1 if this process has no children.
2717 int
2718 wait(int *status)
2719 {
2720   struct proc *p;
2721   int havekids, pid;
2722 
2723   acquire(&ptable.lock);
2724   for(;;){
2725     // Scan through table looking for exited children.
2726     havekids = 0;
2727     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2728       if(p->parent != proc)
2729         continue;
2730       havekids = 1;
2731       if(p->state == ZOMBIE){
2732         // Found one.
2733         pid = p->pid;
2734         kfree(p->kstack);
2735         p->kstack = 0;
2736         freevm(p->pgdir);
2737         p->pid = 0;
2738         p->parent = 0;
2739         p->name[0] = 0;
2740         p->killed = 0;
2741         p->state = UNUSED;
2742         if(status!=0)
2743           *status=(p->exitStat);
2744         release(&ptable.lock);
2745         return pid;
2746       }
2747     }
2748 
2749 
2750     // No point waiting if we don't have any children.
2751     if(!havekids || proc->killed){
2752       release(&ptable.lock);
2753       return -1;
2754     }
2755 
2756     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2757     sleep(proc, &ptable.lock);  //DOC: wait-sleep
2758   }
2759 }
2760 
2761 
2762 int rando(void)
2763 {
2764   static int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
2765   int b;
2766   b  = ((z1 << 6) ^ z1) >> 13;
2767   z1 = ((z1 & 4294967294U) << 18) ^ b;
2768   b  = ((z2 << 2) ^ z2) >> 27;
2769   z2 = ((z2 & 4294967288U) << 2) ^ b;
2770   b  = ((z3 << 13) ^ z3) >> 21;
2771   z3 = ((z3 & 4294967280U) << 7) ^ b;
2772   b  = ((z4 << 3) ^ z4) >> 12;
2773   z4 = ((z4 & 4294967168U) << 13) ^ b;
2774   int ret= (z1 ^ z2 ^ z3 ^ z4);
2775   if(ret<0)
2776     ret*=(-1);
2777   return ret;
2778 }
2779 
2780 
2781 
2782 
2783 
2784 
2785 
2786 
2787 
2788 
2789 
2790 
2791 
2792 
2793 
2794 
2795 
2796 
2797 
2798 
2799 
2800 // Per-CPU process scheduler.
2801 // Each CPU calls scheduler() after setting itself up.
2802 // Scheduler never returns.  It loops, doing:
2803 //  - choose a process to run
2804 //  - swtch to start running that process
2805 //  - eventually that process transfers control
2806 //      via swtch back to the scheduler.
2807 void
2808 scheduler(void)
2809 {
2810   struct proc *p;
2811 
2812   for(;;){
2813     // Enable interrupts on this processor.
2814     sti();
2815 
2816     // Loop over process table looking for process to run.
2817     acquire(&ptable.lock);
2818 
2819     int totalNumTickets=0;
2820 
2821 
2822     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2823         if(p->state == RUNNABLE || p->state == RUNNING)
2824           totalNumTickets+= p->ntickets;
2825     }
2826 
2827     if(totalNumTickets<=0){
2828       release(&ptable.lock);
2829       continue;
2830     }
2831 
2832     //get random number
2833     int ran = rando();
2834     ran = ran % totalNumTickets;
2835 
2836     int  preSumNtic=0;
2837 
2838     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2839       if(p->state != RUNNABLE)
2840         continue;
2841 
2842       if(!(ran>=preSumNtic && ran <= (preSumNtic+ p-> ntickets) -1 )){
2843         preSumNtic+=p-> ntickets;
2844         continue;
2845       }
2846 
2847 
2848 
2849 
2850       // Switch to chosen process.  It is the process's job
2851       // to release ptable.lock and then reacquire it
2852       // before jumping back to us.
2853       proc = p;
2854       switchuvm(p);
2855       p->state = RUNNING;
2856       swtch(&cpu->scheduler, p->context);
2857       switchkvm();
2858 
2859       // Process is done running for now.
2860       // It should have changed its p->state before coming back.
2861       proc = 0;
2862 
2863       if(p->ntickets>1)
2864         priority(p->ntickets -1);
2865 
2866       break;
2867     }
2868     release(&ptable.lock);
2869 
2870   }
2871 }
2872 
2873 // Enter scheduler.  Must hold only ptable.lock
2874 // and have changed proc->state. Saves and restores
2875 // intena because intena is a property of this
2876 // kernel thread, not this CPU. It should
2877 // be proc->intena and proc->ncli, but that would
2878 // break in the few places where a lock is held but
2879 // there's no process.
2880 void
2881 sched(void)
2882 {
2883   int intena;
2884 
2885   if(!holding(&ptable.lock))
2886     panic("sched ptable.lock");
2887   if(cpu->ncli != 1)
2888     panic("sched locks");
2889   if(proc->state == RUNNING)
2890     panic("sched running");
2891   if(readeflags()&FL_IF)
2892     panic("sched interruptible");
2893   intena = cpu->intena;
2894   swtch(&proc->context, cpu->scheduler);
2895   cpu->intena = intena;
2896 }
2897 
2898 
2899 
2900 // Give up the CPU for one scheduling round.
2901 void
2902 yield(void)
2903 {
2904   acquire(&ptable.lock);  //DOC: yieldlock
2905   proc->state = RUNNABLE;
2906   sched();
2907   release(&ptable.lock);
2908 }
2909 
2910 // A fork child's very first scheduling by scheduler()
2911 // will swtch here.  "Return" to user space.
2912 void
2913 forkret(void)
2914 {
2915   static int first = 1;
2916   // Still holding ptable.lock from scheduler.
2917   release(&ptable.lock);
2918 
2919   if (first) {
2920     // Some initialization functions must be run in the context
2921     // of a regular process (e.g., they call sleep), and thus cannot
2922     // be run from main().
2923     first = 0;
2924     iinit(ROOTDEV);
2925     initlog(ROOTDEV);
2926   }
2927 
2928   // Return to "caller", actually trapret (see allocproc).
2929 }
2930 
2931 // Atomically release lock and sleep on chan.
2932 // Reacquires lock when awakened.
2933 void
2934 sleep(void *chan, struct spinlock *lk)
2935 {
2936   if(proc == 0)
2937     panic("sleep");
2938 
2939   if(lk == 0)
2940     panic("sleep without lk");
2941 
2942   // Must acquire ptable.lock in order to
2943   // change p->state and then call sched.
2944   // Once we hold ptable.lock, we can be
2945   // guaranteed that we won't miss any wakeup
2946   // (wakeup runs with ptable.lock locked),
2947   // so it's okay to release lk.
2948   if(lk != &ptable.lock){  //DOC: sleeplock0
2949     acquire(&ptable.lock);  //DOC: sleeplock1
2950     release(lk);
2951   }
2952 
2953   // Go to sleep.
2954   proc->chan = chan;
2955   proc->state = SLEEPING;
2956 
2957   if(proc->ntickets<91)
2958   priority(proc->ntickets + 10);
2959 
2960   sched();
2961 
2962   // Tidy up.
2963   proc->chan = 0;
2964 
2965   // Reacquire original lock.
2966   if(lk != &ptable.lock){  //DOC: sleeplock2
2967     release(&ptable.lock);
2968     acquire(lk);
2969   }
2970 }
2971 
2972 
2973 
2974 
2975 
2976 
2977 
2978 
2979 
2980 
2981 
2982 
2983 
2984 
2985 
2986 
2987 
2988 
2989 
2990 
2991 
2992 
2993 
2994 
2995 
2996 
2997 
2998 
2999 
3000 // Wake up all processes sleeping on chan.
3001 // The ptable lock must be held.
3002 static void
3003 wakeup1(void *chan)
3004 {
3005   struct proc *p;
3006 
3007   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
3008     if(p->state == SLEEPING && p->chan == chan)
3009       p->state = RUNNABLE;
3010 }
3011 
3012 // Wake up all processes sleeping on chan.
3013 void
3014 wakeup(void *chan)
3015 {
3016   acquire(&ptable.lock);
3017   wakeup1(chan);
3018   release(&ptable.lock);
3019 }
3020 
3021 // Kill the process with the given pid.
3022 // Process won't exit until it returns
3023 // to user space (see trap in trap.c).
3024 int
3025 kill(int pid)
3026 {
3027   struct proc *p;
3028 
3029   acquire(&ptable.lock);
3030   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3031     if(p->pid == pid){
3032       p->killed = 1;
3033       // Wake process from sleep if necessary.
3034       if(p->state == SLEEPING)
3035         p->state = RUNNABLE;
3036       release(&ptable.lock);
3037       return 0;
3038     }
3039   }
3040   release(&ptable.lock);
3041   return -1;
3042 }
3043 
3044 
3045 
3046 
3047 
3048 
3049 
3050 // Print a process listing to console.  For debugging.
3051 // Runs when user types ^P on console.
3052 // No lock to avoid wedging a stuck machine further.
3053 void
3054 procdump(void)
3055 {
3056   static char *states[] = {
3057   [UNUSED]    "unused",
3058   [EMBRYO]    "embryo",
3059   [SLEEPING]  "sleep ",
3060   [RUNNABLE]  "runble",
3061   [RUNNING]   "run   ",
3062   [ZOMBIE]    "zombie"
3063   };
3064   int i;
3065   struct proc *p;
3066   char *state;
3067   uint pc[10];
3068 
3069   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3070     if(p->state == UNUSED)
3071       continue;
3072     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
3073       state = states[p->state];
3074     else
3075       state = "???";
3076     cprintf("%d %s %s", p->pid, state, p->name);
3077     if(p->state == SLEEPING){
3078       getcallerpcs((uint*)p->context->ebp+2, pc);
3079       for(i=0; i<10 && pc[i] != 0; i++)
3080         cprintf(" %p", pc[i]);
3081     }
3082     cprintf("\n");
3083   }
3084 }
3085 
3086 
3087 
3088 
3089 
3090 
3091 
3092 
3093 
3094 
3095 
3096 
3097 
3098 
3099 
