7350 // Multiprocessor support
7351 // Search memory for MP description structures.
7352 // http://developer.intel.com/design/pentium/datashts/24201606.pdf
7353 
7354 #include "types.h"
7355 #include "defs.h"
7356 #include "param.h"
7357 #include "memlayout.h"
7358 #include "mp.h"
7359 #include "x86.h"
7360 #include "mmu.h"
7361 #include "proc.h"
7362 
7363 struct cpu cpus[NCPU];
7364 int ismp;
7365 int ncpu;
7366 uchar ioapicid;
7367 
7368 static uchar
7369 sum(uchar *addr, int len)
7370 {
7371   int i, sum;
7372 
7373   sum = 0;
7374   for(i=0; i<len; i++)
7375     sum += addr[i];
7376   return sum;
7377 }
7378 
7379 // Look for an MP structure in the len bytes at addr.
7380 static struct mp*
7381 mpsearch1(uint a, int len)
7382 {
7383   uchar *e, *p, *addr;
7384 
7385   addr = P2V(a);
7386   e = addr+len;
7387   for(p = addr; p < e; p += sizeof(struct mp))
7388     if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
7389       return (struct mp*)p;
7390   return 0;
7391 }
7392 
7393 
7394 
7395 
7396 
7397 
7398 
7399 
7400 // Search for the MP Floating Pointer Structure, which according to the
7401 // spec is in one of the following three locations:
7402 // 1) in the first KB of the EBDA;
7403 // 2) in the last KB of system base memory;
7404 // 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
7405 static struct mp*
7406 mpsearch(void)
7407 {
7408   uchar *bda;
7409   uint p;
7410   struct mp *mp;
7411 
7412   bda = (uchar *) P2V(0x400);
7413   if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
7414     if((mp = mpsearch1(p, 1024)))
7415       return mp;
7416   } else {
7417     p = ((bda[0x14]<<8)|bda[0x13])*1024;
7418     if((mp = mpsearch1(p-1024, 1024)))
7419       return mp;
7420   }
7421   return mpsearch1(0xF0000, 0x10000);
7422 }
7423 
7424 // Search for an MP configuration table.  For now,
7425 // don't accept the default configurations (physaddr == 0).
7426 // Check for correct signature, calculate the checksum and,
7427 // if correct, check the version.
7428 // To do: check extended table checksum.
7429 static struct mpconf*
7430 mpconfig(struct mp **pmp)
7431 {
7432   struct mpconf *conf;
7433   struct mp *mp;
7434 
7435   if((mp = mpsearch()) == 0 || mp->physaddr == 0)
7436     return 0;
7437   conf = (struct mpconf*) P2V((uint) mp->physaddr);
7438   if(memcmp(conf, "PCMP", 4) != 0)
7439     return 0;
7440   if(conf->version != 1 && conf->version != 4)
7441     return 0;
7442   if(sum((uchar*)conf, conf->length) != 0)
7443     return 0;
7444   *pmp = mp;
7445   return conf;
7446 }
7447 
7448 
7449 
7450 void
7451 mpinit(void)
7452 {
7453   uchar *p, *e;
7454   struct mp *mp;
7455   struct mpconf *conf;
7456   struct mpproc *proc;
7457   struct mpioapic *ioapic;
7458 
7459   if((conf = mpconfig(&mp)) == 0)
7460     return;
7461   ismp = 1;
7462   lapic = (uint*)conf->lapicaddr;
7463   for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){
7464     switch(*p){
7465     case MPPROC:
7466       proc = (struct mpproc*)p;
7467       if(ncpu < NCPU) {
7468         cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
7469         ncpu++;
7470       }
7471       p += sizeof(struct mpproc);
7472       continue;
7473     case MPIOAPIC:
7474       ioapic = (struct mpioapic*)p;
7475       ioapicid = ioapic->apicno;
7476       p += sizeof(struct mpioapic);
7477       continue;
7478     case MPBUS:
7479     case MPIOINTR:
7480     case MPLINTR:
7481       p += 8;
7482       continue;
7483     default:
7484       ismp = 0;
7485       break;
7486     }
7487   }
7488   if(!ismp){
7489     // Didn't like what we found; fall back to no MP.
7490     ncpu = 1;
7491     lapic = 0;
7492     ioapicid = 0;
7493     return;
7494   }
7495 
7496 
7497 
7498 
7499 
7500   if(mp->imcrp){
7501     // Bochs doesn't support IMCR, so this doesn't run on Bochs.
7502     // But it would on real hardware.
7503     outb(0x22, 0x70);   // Select IMCR
7504     outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
7505   }
7506 }
7507 
7508 
7509 
7510 
7511 
7512 
7513 
7514 
7515 
7516 
7517 
7518 
7519 
7520 
7521 
7522 
7523 
7524 
7525 
7526 
7527 
7528 
7529 
7530 
7531 
7532 
7533 
7534 
7535 
7536 
7537 
7538 
7539 
7540 
7541 
7542 
7543 
7544 
7545 
7546 
7547 
7548 
7549 
