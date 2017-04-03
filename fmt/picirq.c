8000 // Intel 8259A programmable interrupt controllers.
8001 
8002 #include "types.h"
8003 #include "x86.h"
8004 #include "traps.h"
8005 
8006 // I/O Addresses of the two programmable interrupt controllers
8007 #define IO_PIC1         0x20    // Master (IRQs 0-7)
8008 #define IO_PIC2         0xA0    // Slave (IRQs 8-15)
8009 
8010 #define IRQ_SLAVE       2       // IRQ at which slave connects to master
8011 
8012 // Current IRQ mask.
8013 // Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
8014 static ushort irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);
8015 
8016 static void
8017 picsetmask(ushort mask)
8018 {
8019   irqmask = mask;
8020   outb(IO_PIC1+1, mask);
8021   outb(IO_PIC2+1, mask >> 8);
8022 }
8023 
8024 void
8025 picenable(int irq)
8026 {
8027   picsetmask(irqmask & ~(1<<irq));
8028 }
8029 
8030 // Initialize the 8259A interrupt controllers.
8031 void
8032 picinit(void)
8033 {
8034   // mask all interrupts
8035   outb(IO_PIC1+1, 0xFF);
8036   outb(IO_PIC2+1, 0xFF);
8037 
8038   // Set up master (8259A-1)
8039 
8040   // ICW1:  0001g0hi
8041   //    g:  0 = edge triggering, 1 = level triggering
8042   //    h:  0 = cascaded PICs, 1 = master only
8043   //    i:  0 = no ICW4, 1 = ICW4 required
8044   outb(IO_PIC1, 0x11);
8045 
8046   // ICW2:  Vector offset
8047   outb(IO_PIC1+1, T_IRQ0);
8048 
8049 
8050   // ICW3:  (master PIC) bit mask of IR lines connected to slaves
8051   //        (slave PIC) 3-bit # of slave's connection to master
8052   outb(IO_PIC1+1, 1<<IRQ_SLAVE);
8053 
8054   // ICW4:  000nbmap
8055   //    n:  1 = special fully nested mode
8056   //    b:  1 = buffered mode
8057   //    m:  0 = slave PIC, 1 = master PIC
8058   //      (ignored when b is 0, as the master/slave role
8059   //      can be hardwired).
8060   //    a:  1 = Automatic EOI mode
8061   //    p:  0 = MCS-80/85 mode, 1 = intel x86 mode
8062   outb(IO_PIC1+1, 0x3);
8063 
8064   // Set up slave (8259A-2)
8065   outb(IO_PIC2, 0x11);                  // ICW1
8066   outb(IO_PIC2+1, T_IRQ0 + 8);      // ICW2
8067   outb(IO_PIC2+1, IRQ_SLAVE);           // ICW3
8068   // NB Automatic EOI mode doesn't tend to work on the slave.
8069   // Linux source code says it's "to be investigated".
8070   outb(IO_PIC2+1, 0x3);                 // ICW4
8071 
8072   // OCW3:  0ef01prs
8073   //   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
8074   //    p:  0 = no polling, 1 = polling mode
8075   //   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
8076   outb(IO_PIC1, 0x68);             // clear specific mask
8077   outb(IO_PIC1, 0x0a);             // read IRR by default
8078 
8079   outb(IO_PIC2, 0x68);             // OCW3
8080   outb(IO_PIC2, 0x0a);             // OCW3
8081 
8082   if(irqmask != 0xFFFF)
8083     picsetmask(irqmask);
8084 }
8085 
8086 
8087 
8088 
8089 
8090 
8091 
8092 
8093 
8094 
8095 
8096 
8097 
8098 
8099 
8100 // Blank page.
8101 
8102 
8103 
8104 
8105 
8106 
8107 
8108 
8109 
8110 
8111 
8112 
8113 
8114 
8115 
8116 
8117 
8118 
8119 
8120 
8121 
8122 
8123 
8124 
8125 
8126 
8127 
8128 
8129 
8130 
8131 
8132 
8133 
8134 
8135 
8136 
8137 
8138 
8139 
8140 
8141 
8142 
8143 
8144 
8145 
8146 
8147 
8148 
8149 
