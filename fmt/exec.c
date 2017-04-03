6750 #include "types.h"
6751 #include "traps.h"
6752 #include "param.h"
6753 #include "memlayout.h"
6754 #include "mmu.h"
6755 #include "proc.h"
6756 #include "defs.h"
6757 #include "syscall.h"
6758 #include "x86.h"
6759 #include "elf.h"
6760 
6761 void
6762 pseudo_main(int (*entry)(int, char**), int argc, char **argv)
6763 {
6764     int stat =entry(argc, argv);
6765 
6766      asm("pushl %%eax\n"
6767     "pushl %%eax\n"
6768     "movl $2, %%eax\n"
6769     "int %1" :: "a"(stat), "i" (T_SYSCALL));
6770 
6771 }
6772 
6773 int
6774 exec(char *path, char **argv)
6775 {
6776   char *s, *last;
6777   int i, off;
6778   uint argc, sz, sp, ustack[3+MAXARG+1];
6779   uint pointer_pseudo_main;
6780   struct elfhdr elf;
6781   struct inode *ip;
6782   struct proghdr ph;
6783   pde_t *pgdir, *oldpgdir;
6784 
6785   begin_op();
6786 
6787   if((ip = namei(path)) == 0){
6788     end_op();
6789     return -1;
6790   }
6791   ilock(ip);
6792   pgdir = 0;
6793 
6794   // Check ELF header
6795   if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
6796     goto bad;
6797   if(elf.magic != ELF_MAGIC)
6798     goto bad;
6799 
6800   if((pgdir = setupkvm()) == 0)
6801     goto bad;
6802 
6803   // Load program into memory.
6804   sz = 0;
6805   for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
6806     if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
6807       goto bad;
6808     if(ph.type != ELF_PROG_LOAD)
6809       continue;
6810     if(ph.memsz < ph.filesz)
6811       goto bad;
6812     if(ph.vaddr + ph.memsz < ph.vaddr)
6813       goto bad;
6814     if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
6815       goto bad;
6816     if(ph.vaddr % PGSIZE != 0)
6817       goto bad;
6818     if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
6819       goto bad;
6820   }
6821   iunlockput(ip);
6822   end_op();
6823   ip = 0;
6824 
6825   pointer_pseudo_main = sz;
6826 
6827 
6828   // Allocate two pages at the next page boundary.
6829   // Make the first inaccessible.  Use the second as the user stack.
6830   sz = PGROUNDUP(sz);
6831   if((sz = allocuvm(pgdir, sz, sz + 3*PGSIZE)) == 0)
6832     goto bad;
6833   clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
6834 
6835   if (copyout(pgdir, pointer_pseudo_main, pseudo_main, (uint)exec - (uint)pseudo_main) < 0)
6836     goto bad;
6837 
6838   sp = sz;
6839 
6840   // Push argument strings, prepare rest of stack in ustack.
6841   for(argc = 0; argv[argc]; argc++) {
6842     if(argc >= MAXARG)
6843       goto bad;
6844     sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
6845     if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
6846       goto bad;
6847     ustack[3+argc] = sp;
6848   }
6849   ustack[3+argc] = 0;
6850   ustack[0] = 0xffffffff;  // fake return PC
6851   ustack[1]=elf.entry;
6852   ustack[2] = argc;
6853   ustack[3] = sp - (argc+1)*4;  // argv pointer
6854 
6855   sp -= (3+argc+1) * 4;
6856   if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
6857     goto bad;
6858 
6859   // Save program name for debugging.
6860   for(last=s=path; *s; s++)
6861     if(*s == '/')
6862       last = s+1;
6863   safestrcpy(proc->name, last, sizeof(proc->name));
6864 
6865   // Commit to the user image.
6866   oldpgdir = proc->pgdir;
6867   proc->pgdir = pgdir;
6868   proc->sz = sz;
6869   proc->tf->eip = pointer_pseudo_main;  // main
6870   proc->tf->esp = sp;
6871   switchuvm(proc);
6872   freevm(oldpgdir);
6873   return 0;
6874 
6875  bad:
6876   if(pgdir)
6877     freevm(pgdir);
6878   if(ip){
6879     iunlockput(ip);
6880     end_op();
6881   }
6882   return -1;
6883 }
6884 
6885 
6886 
6887 
6888 
6889 
6890 
6891 
6892 
6893 
6894 
6895 
6896 
6897 
6898 
6899 
