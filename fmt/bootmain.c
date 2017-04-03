9800 // Boot loader.
9801 //
9802 // Part of the boot block, along with bootasm.S, which calls bootmain().
9803 // bootasm.S has put the processor into protected 32-bit mode.
9804 // bootmain() loads an ELF kernel image from the disk starting at
9805 // sector 1 and then jumps to the kernel entry routine.
9806 
9807 #include "types.h"
9808 #include "elf.h"
9809 #include "x86.h"
9810 #include "memlayout.h"
9811 
9812 #define SECTSIZE  512
9813 
9814 void readseg(uchar*, uint, uint);
9815 
9816 void
9817 bootmain(void)
9818 {
9819   struct elfhdr *elf;
9820   struct proghdr *ph, *eph;
9821   void (*entry)(void);
9822   uchar* pa;
9823 
9824   elf = (struct elfhdr*)0x10000;  // scratch space
9825 
9826   // Read 1st page off disk
9827   readseg((uchar*)elf, 4096, 0);
9828 
9829   // Is this an ELF executable?
9830   if(elf->magic != ELF_MAGIC)
9831     return;  // let bootasm.S handle error
9832 
9833   // Load each program segment (ignores ph flags).
9834   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9835   eph = ph + elf->phnum;
9836   for(; ph < eph; ph++){
9837     pa = (uchar*)ph->paddr;
9838     readseg(pa, ph->filesz, ph->off);
9839     if(ph->memsz > ph->filesz)
9840       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9841   }
9842 
9843   // Call the entry point from the ELF header.
9844   // Does not return!
9845   entry = (void(*)(void))(elf->entry);
9846   entry();
9847 }
9848 
9849 
9850 void
9851 waitdisk(void)
9852 {
9853   // Wait for disk ready.
9854   while((inb(0x1F7) & 0xC0) != 0x40)
9855     ;
9856 }
9857 
9858 // Read a single sector at offset into dst.
9859 void
9860 readsect(void *dst, uint offset)
9861 {
9862   // Issue command.
9863   waitdisk();
9864   outb(0x1F2, 1);   // count = 1
9865   outb(0x1F3, offset);
9866   outb(0x1F4, offset >> 8);
9867   outb(0x1F5, offset >> 16);
9868   outb(0x1F6, (offset >> 24) | 0xE0);
9869   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9870 
9871   // Read data.
9872   waitdisk();
9873   insl(0x1F0, dst, SECTSIZE/4);
9874 }
9875 
9876 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9877 // Might copy more than asked.
9878 void
9879 readseg(uchar* pa, uint count, uint offset)
9880 {
9881   uchar* epa;
9882 
9883   epa = pa + count;
9884 
9885   // Round down to sector boundary.
9886   pa -= offset % SECTSIZE;
9887 
9888   // Translate from bytes to sectors; kernel starts at sector 1.
9889   offset = (offset / SECTSIZE) + 1;
9890 
9891   // If this is too slow, we could read lots of sectors at a time.
9892   // We'd write more to memory than asked, but it doesn't matter --
9893   // we load in increasing order.
9894   for(; pa < epa; pa += SECTSIZE, offset++)
9895     readsect(pa, offset);
9896 }
9897 
9898 
9899 
