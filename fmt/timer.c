8750 // Intel 8253/8254/82C54 Programmable Interval Timer (PIT).
8751 // Only used on uniprocessors;
8752 // SMP machines use the local APIC timer.
8753 
8754 #include "types.h"
8755 #include "defs.h"
8756 #include "traps.h"
8757 #include "x86.h"
8758 
8759 #define IO_TIMER1       0x040           // 8253 Timer #1
8760 
8761 // Frequency of all three count-down timers;
8762 // (TIMER_FREQ/freq) is the appropriate count
8763 // to generate a frequency of freq Hz.
8764 
8765 #define TIMER_FREQ      1193182
8766 #define TIMER_DIV(x)    ((TIMER_FREQ+(x)/2)/(x))
8767 
8768 #define TIMER_MODE      (IO_TIMER1 + 3) // timer mode port
8769 #define TIMER_SEL0      0x00    // select counter 0
8770 #define TIMER_RATEGEN   0x04    // mode 2, rate generator
8771 #define TIMER_16BIT     0x30    // r/w counter 16 bits, LSB first
8772 
8773 void
8774 timerinit(void)
8775 {
8776   // Interrupt 100 times/sec.
8777   outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
8778   outb(IO_TIMER1, TIMER_DIV(100) % 256);
8779   outb(IO_TIMER1, TIMER_DIV(100) / 256);
8780   picenable(IRQ_TIMER);
8781 }
8782 
8783 
8784 
8785 
8786 
8787 
8788 
8789 
8790 
8791 
8792 
8793 
8794 
8795 
8796 
8797 
8798 
8799 
