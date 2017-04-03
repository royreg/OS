8800 // Intel 8250 serial port (UART).
8801 
8802 #include "types.h"
8803 #include "defs.h"
8804 #include "param.h"
8805 #include "traps.h"
8806 #include "spinlock.h"
8807 #include "sleeplock.h"
8808 #include "fs.h"
8809 #include "file.h"
8810 #include "mmu.h"
8811 #include "proc.h"
8812 #include "x86.h"
8813 
8814 #define COM1    0x3f8
8815 
8816 static int uart;    // is there a uart?
8817 
8818 void
8819 uartinit(void)
8820 {
8821   char *p;
8822 
8823   // Turn off the FIFO
8824   outb(COM1+2, 0);
8825 
8826   // 9600 baud, 8 data bits, 1 stop bit, parity off.
8827   outb(COM1+3, 0x80);    // Unlock divisor
8828   outb(COM1+0, 115200/9600);
8829   outb(COM1+1, 0);
8830   outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
8831   outb(COM1+4, 0);
8832   outb(COM1+1, 0x01);    // Enable receive interrupts.
8833 
8834   // If status is 0xFF, no serial port.
8835   if(inb(COM1+5) == 0xFF)
8836     return;
8837   uart = 1;
8838 
8839   // Acknowledge pre-existing interrupt conditions;
8840   // enable interrupts.
8841   inb(COM1+2);
8842   inb(COM1+0);
8843   picenable(IRQ_COM1);
8844   ioapicenable(IRQ_COM1, 0);
8845 
8846   // Announce that we're here.
8847   for(p="xv6...\n"; *p; p++)
8848     uartputc(*p);
8849 }
8850 void
8851 uartputc(int c)
8852 {
8853   int i;
8854 
8855   if(!uart)
8856     return;
8857   for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
8858     microdelay(10);
8859   outb(COM1+0, c);
8860 }
8861 
8862 static int
8863 uartgetc(void)
8864 {
8865   if(!uart)
8866     return -1;
8867   if(!(inb(COM1+5) & 0x01))
8868     return -1;
8869   return inb(COM1+0);
8870 }
8871 
8872 void
8873 uartintr(void)
8874 {
8875   consoleintr(uartgetc);
8876 }
8877 
8878 
8879 
8880 
8881 
8882 
8883 
8884 
8885 
8886 
8887 
8888 
8889 
8890 
8891 
8892 
8893 
8894 
8895 
8896 
8897 
8898 
8899 
