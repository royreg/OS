9700 #include "asm.h"
9701 #include "memlayout.h"
9702 #include "mmu.h"
9703 
9704 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9705 # The BIOS loads this code from the first sector of the hard disk into
9706 # memory at physical address 0x7c00 and starts executing in real mode
9707 # with %cs=0 %ip=7c00.
9708 
9709 .code16                       # Assemble for 16-bit mode
9710 .globl start
9711 start:
9712   cli                         # BIOS enabled interrupts; disable
9713 
9714   # Zero data segment registers DS, ES, and SS.
9715   xorw    %ax,%ax             # Set %ax to zero
9716   movw    %ax,%ds             # -> Data Segment
9717   movw    %ax,%es             # -> Extra Segment
9718   movw    %ax,%ss             # -> Stack Segment
9719 
9720   # Physical address line A20 is tied to zero so that the first PCs
9721   # with 2 MB would run software that assumed 1 MB.  Undo that.
9722 seta20.1:
9723   inb     $0x64,%al               # Wait for not busy
9724   testb   $0x2,%al
9725   jnz     seta20.1
9726 
9727   movb    $0xd1,%al               # 0xd1 -> port 0x64
9728   outb    %al,$0x64
9729 
9730 seta20.2:
9731   inb     $0x64,%al               # Wait for not busy
9732   testb   $0x2,%al
9733   jnz     seta20.2
9734 
9735   movb    $0xdf,%al               # 0xdf -> port 0x60
9736   outb    %al,$0x60
9737 
9738   # Switch from real to protected mode.  Use a bootstrap GDT that makes
9739   # virtual addresses map directly to physical addresses so that the
9740   # effective memory map doesn't change during the transition.
9741   lgdt    gdtdesc
9742   movl    %cr0, %eax
9743   orl     $CR0_PE, %eax
9744   movl    %eax, %cr0
9745 
9746 
9747 
9748 
9749 
9750   # Complete the transition to 32-bit protected mode by using a long jmp
9751   # to reload %cs and %eip.  The segment descriptors are set up with no
9752   # translation, so that the mapping is still the identity mapping.
9753   ljmp    $(SEG_KCODE<<3), $start32
9754 
9755 .code32  # Tell assembler to generate 32-bit code now.
9756 start32:
9757   # Set up the protected-mode data segment registers
9758   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
9759   movw    %ax, %ds                # -> DS: Data Segment
9760   movw    %ax, %es                # -> ES: Extra Segment
9761   movw    %ax, %ss                # -> SS: Stack Segment
9762   movw    $0, %ax                 # Zero segments not ready for use
9763   movw    %ax, %fs                # -> FS
9764   movw    %ax, %gs                # -> GS
9765 
9766   # Set up the stack pointer and call into C.
9767   movl    $start, %esp
9768   call    bootmain
9769 
9770   # If bootmain returns (it shouldn't), trigger a Bochs
9771   # breakpoint if running under Bochs, then loop.
9772   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
9773   movw    %ax, %dx
9774   outw    %ax, %dx
9775   movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
9776   outw    %ax, %dx
9777 spin:
9778   jmp     spin
9779 
9780 # Bootstrap GDT
9781 .p2align 2                                # force 4 byte alignment
9782 gdt:
9783   SEG_NULLASM                             # null seg
9784   SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
9785   SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
9786 
9787 gdtdesc:
9788   .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
9789   .long   gdt                             # address gdt
9790 
9791 
9792 
9793 
9794 
9795 
9796 
9797 
9798 
9799 
