3400 #include "mmu.h"
3401 
3402   # vectors.S sends all traps here.
3403 .globl alltraps
3404 alltraps:
3405   # Build trap frame.
3406   pushl %ds
3407   pushl %es
3408   pushl %fs
3409   pushl %gs
3410   pushal
3411 
3412   # Set up data and per-cpu segments.
3413   movw $(SEG_KDATA<<3), %ax
3414   movw %ax, %ds
3415   movw %ax, %es
3416   movw $(SEG_KCPU<<3), %ax
3417   movw %ax, %fs
3418   movw %ax, %gs
3419 
3420   # Call trap(tf), where tf=%esp
3421   pushl %esp
3422   call trap
3423   addl $4, %esp
3424 
3425   # Return falls through to trapret...
3426 .globl trapret
3427 trapret:
3428   popal
3429   popl %gs
3430   popl %fs
3431   popl %es
3432   popl %ds
3433   addl $0x8, %esp  # trapno and errcode
3434   iret
3435 
3436 
3437 
3438 
3439 
3440 
3441 
3442 
3443 
3444 
3445 
3446 
3447 
3448 
3449 
