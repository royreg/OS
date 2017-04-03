7550 // The local APIC manages internal (non-I/O) interrupts.
7551 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7552 
7553 #include "param.h"
7554 #include "types.h"
7555 #include "defs.h"
7556 #include "date.h"
7557 #include "memlayout.h"
7558 #include "traps.h"
7559 #include "mmu.h"
7560 #include "x86.h"
7561 #include "proc.h"  // ncpu
7562 
7563 // Local APIC registers, divided by 4 for use as uint[] indices.
7564 #define ID      (0x0020/4)   // ID
7565 #define VER     (0x0030/4)   // Version
7566 #define TPR     (0x0080/4)   // Task Priority
7567 #define EOI     (0x00B0/4)   // EOI
7568 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7569   #define ENABLE     0x00000100   // Unit Enable
7570 #define ESR     (0x0280/4)   // Error Status
7571 #define ICRLO   (0x0300/4)   // Interrupt Command
7572   #define INIT       0x00000500   // INIT/RESET
7573   #define STARTUP    0x00000600   // Startup IPI
7574   #define DELIVS     0x00001000   // Delivery status
7575   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7576   #define DEASSERT   0x00000000
7577   #define LEVEL      0x00008000   // Level triggered
7578   #define BCAST      0x00080000   // Send to all APICs, including self.
7579   #define BUSY       0x00001000
7580   #define FIXED      0x00000000
7581 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7582 #define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7583   #define X1         0x0000000B   // divide counts by 1
7584   #define PERIODIC   0x00020000   // Periodic
7585 #define PCINT   (0x0340/4)   // Performance Counter LVT
7586 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7587 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7588 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7589   #define MASKED     0x00010000   // Interrupt masked
7590 #define TICR    (0x0380/4)   // Timer Initial Count
7591 #define TCCR    (0x0390/4)   // Timer Current Count
7592 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7593 
7594 volatile uint *lapic;  // Initialized in mp.c
7595 
7596 
7597 
7598 
7599 
7600 static void
7601 lapicw(int index, int value)
7602 {
7603   lapic[index] = value;
7604   lapic[ID];  // wait for write to finish, by reading
7605 }
7606 
7607 
7608 
7609 
7610 
7611 
7612 
7613 
7614 
7615 
7616 
7617 
7618 
7619 
7620 
7621 
7622 
7623 
7624 
7625 
7626 
7627 
7628 
7629 
7630 
7631 
7632 
7633 
7634 
7635 
7636 
7637 
7638 
7639 
7640 
7641 
7642 
7643 
7644 
7645 
7646 
7647 
7648 
7649 
7650 void
7651 lapicinit(void)
7652 {
7653   if(!lapic)
7654     return;
7655 
7656   // Enable local APIC; set spurious interrupt vector.
7657   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7658 
7659   // The timer repeatedly counts down at bus frequency
7660   // from lapic[TICR] and then issues an interrupt.
7661   // If xv6 cared more about precise timekeeping,
7662   // TICR would be calibrated using an external time source.
7663   lapicw(TDCR, X1);
7664   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7665   lapicw(TICR, 10000000);
7666 
7667   // Disable logical interrupt lines.
7668   lapicw(LINT0, MASKED);
7669   lapicw(LINT1, MASKED);
7670 
7671   // Disable performance counter overflow interrupts
7672   // on machines that provide that interrupt entry.
7673   if(((lapic[VER]>>16) & 0xFF) >= 4)
7674     lapicw(PCINT, MASKED);
7675 
7676   // Map error interrupt to IRQ_ERROR.
7677   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7678 
7679   // Clear error status register (requires back-to-back writes).
7680   lapicw(ESR, 0);
7681   lapicw(ESR, 0);
7682 
7683   // Ack any outstanding interrupts.
7684   lapicw(EOI, 0);
7685 
7686   // Send an Init Level De-Assert to synchronise arbitration ID's.
7687   lapicw(ICRHI, 0);
7688   lapicw(ICRLO, BCAST | INIT | LEVEL);
7689   while(lapic[ICRLO] & DELIVS)
7690     ;
7691 
7692   // Enable interrupts on the APIC (but not on the processor).
7693   lapicw(TPR, 0);
7694 }
7695 
7696 
7697 
7698 
7699 
7700 int
7701 cpunum(void)
7702 {
7703   int apicid, i;
7704 
7705   // Cannot call cpu when interrupts are enabled:
7706   // result not guaranteed to last long enough to be used!
7707   // Would prefer to panic but even printing is chancy here:
7708   // almost everything, including cprintf and panic, calls cpu,
7709   // often indirectly through acquire and release.
7710   if(readeflags()&FL_IF){
7711     static int n;
7712     if(n++ == 0)
7713       cprintf("cpu called from %x with interrupts enabled\n",
7714         __builtin_return_address(0));
7715   }
7716 
7717   if (!lapic)
7718     return 0;
7719 
7720   apicid = lapic[ID] >> 24;
7721   for (i = 0; i < ncpu; ++i) {
7722     if (cpus[i].apicid == apicid)
7723       return i;
7724   }
7725   panic("unknown apicid\n");
7726 }
7727 
7728 // Acknowledge interrupt.
7729 void
7730 lapiceoi(void)
7731 {
7732   if(lapic)
7733     lapicw(EOI, 0);
7734 }
7735 
7736 // Spin for a given number of microseconds.
7737 // On real hardware would want to tune this dynamically.
7738 void
7739 microdelay(int us)
7740 {
7741 }
7742 
7743 
7744 
7745 
7746 
7747 
7748 
7749 
7750 #define CMOS_PORT    0x70
7751 #define CMOS_RETURN  0x71
7752 
7753 // Start additional processor running entry code at addr.
7754 // See Appendix B of MultiProcessor Specification.
7755 void
7756 lapicstartap(uchar apicid, uint addr)
7757 {
7758   int i;
7759   ushort *wrv;
7760 
7761   // "The BSP must initialize CMOS shutdown code to 0AH
7762   // and the warm reset vector (DWORD based at 40:67) to point at
7763   // the AP startup code prior to the [universal startup algorithm]."
7764   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7765   outb(CMOS_PORT+1, 0x0A);
7766   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7767   wrv[0] = 0;
7768   wrv[1] = addr >> 4;
7769 
7770   // "Universal startup algorithm."
7771   // Send INIT (level-triggered) interrupt to reset other CPU.
7772   lapicw(ICRHI, apicid<<24);
7773   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7774   microdelay(200);
7775   lapicw(ICRLO, INIT | LEVEL);
7776   microdelay(100);    // should be 10ms, but too slow in Bochs!
7777 
7778   // Send startup IPI (twice!) to enter code.
7779   // Regular hardware is supposed to only accept a STARTUP
7780   // when it is in the halted state due to an INIT.  So the second
7781   // should be ignored, but it is part of the official Intel algorithm.
7782   // Bochs complains about the second one.  Too bad for Bochs.
7783   for(i = 0; i < 2; i++){
7784     lapicw(ICRHI, apicid<<24);
7785     lapicw(ICRLO, STARTUP | (addr>>12));
7786     microdelay(200);
7787   }
7788 }
7789 
7790 
7791 
7792 
7793 
7794 
7795 
7796 
7797 
7798 
7799 
7800 #define CMOS_STATA   0x0a
7801 #define CMOS_STATB   0x0b
7802 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7803 
7804 #define SECS    0x00
7805 #define MINS    0x02
7806 #define HOURS   0x04
7807 #define DAY     0x07
7808 #define MONTH   0x08
7809 #define YEAR    0x09
7810 
7811 static uint cmos_read(uint reg)
7812 {
7813   outb(CMOS_PORT,  reg);
7814   microdelay(200);
7815 
7816   return inb(CMOS_RETURN);
7817 }
7818 
7819 static void fill_rtcdate(struct rtcdate *r)
7820 {
7821   r->second = cmos_read(SECS);
7822   r->minute = cmos_read(MINS);
7823   r->hour   = cmos_read(HOURS);
7824   r->day    = cmos_read(DAY);
7825   r->month  = cmos_read(MONTH);
7826   r->year   = cmos_read(YEAR);
7827 }
7828 
7829 // qemu seems to use 24-hour GWT and the values are BCD encoded
7830 void cmostime(struct rtcdate *r)
7831 {
7832   struct rtcdate t1, t2;
7833   int sb, bcd;
7834 
7835   sb = cmos_read(CMOS_STATB);
7836 
7837   bcd = (sb & (1 << 2)) == 0;
7838 
7839   // make sure CMOS doesn't modify time while we read it
7840   for(;;) {
7841     fill_rtcdate(&t1);
7842     if(cmos_read(CMOS_STATA) & CMOS_UIP)
7843         continue;
7844     fill_rtcdate(&t2);
7845     if(memcmp(&t1, &t2, sizeof(t1)) == 0)
7846       break;
7847   }
7848 
7849 
7850   // convert
7851   if(bcd) {
7852 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7853     CONV(second);
7854     CONV(minute);
7855     CONV(hour  );
7856     CONV(day   );
7857     CONV(month );
7858     CONV(year  );
7859 #undef     CONV
7860   }
7861 
7862   *r = t1;
7863   r->year += 2000;
7864 }
7865 
7866 
7867 
7868 
7869 
7870 
7871 
7872 
7873 
7874 
7875 
7876 
7877 
7878 
7879 
7880 
7881 
7882 
7883 
7884 
7885 
7886 
7887 
7888 
7889 
7890 
7891 
7892 
7893 
7894 
7895 
7896 
7897 
7898 
7899 
