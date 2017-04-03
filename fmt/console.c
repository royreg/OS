8350 // Console input and output.
8351 // Input is from the keyboard or serial port.
8352 // Output is written to the screen and serial port.
8353 
8354 #include "types.h"
8355 #include "defs.h"
8356 #include "param.h"
8357 #include "traps.h"
8358 #include "spinlock.h"
8359 #include "sleeplock.h"
8360 #include "fs.h"
8361 #include "file.h"
8362 #include "memlayout.h"
8363 #include "mmu.h"
8364 #include "proc.h"
8365 #include "x86.h"
8366 
8367 static void consputc(int);
8368 
8369 static int panicked = 0;
8370 
8371 static struct {
8372   struct spinlock lock;
8373   int locking;
8374 } cons;
8375 
8376 static void
8377 printint(int xx, int base, int sign)
8378 {
8379   static char digits[] = "0123456789abcdef";
8380   char buf[16];
8381   int i;
8382   uint x;
8383 
8384   if(sign && (sign = xx < 0))
8385     x = -xx;
8386   else
8387     x = xx;
8388 
8389   i = 0;
8390   do{
8391     buf[i++] = digits[x % base];
8392   }while((x /= base) != 0);
8393 
8394   if(sign)
8395     buf[i++] = '-';
8396 
8397   while(--i >= 0)
8398     consputc(buf[i]);
8399 }
8400 
8401 
8402 
8403 
8404 
8405 
8406 
8407 
8408 
8409 
8410 
8411 
8412 
8413 
8414 
8415 
8416 
8417 
8418 
8419 
8420 
8421 
8422 
8423 
8424 
8425 
8426 
8427 
8428 
8429 
8430 
8431 
8432 
8433 
8434 
8435 
8436 
8437 
8438 
8439 
8440 
8441 
8442 
8443 
8444 
8445 
8446 
8447 
8448 
8449 
8450 // Print to the console. only understands %d, %x, %p, %s.
8451 void
8452 cprintf(char *fmt, ...)
8453 {
8454   int i, c, locking;
8455   uint *argp;
8456   char *s;
8457 
8458   locking = cons.locking;
8459   if(locking)
8460     acquire(&cons.lock);
8461 
8462   if (fmt == 0)
8463     panic("null fmt");
8464 
8465   argp = (uint*)(void*)(&fmt + 1);
8466   for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
8467     if(c != '%'){
8468       consputc(c);
8469       continue;
8470     }
8471     c = fmt[++i] & 0xff;
8472     if(c == 0)
8473       break;
8474     switch(c){
8475     case 'd':
8476       printint(*argp++, 10, 1);
8477       break;
8478     case 'x':
8479     case 'p':
8480       printint(*argp++, 16, 0);
8481       break;
8482     case 's':
8483       if((s = (char*)*argp++) == 0)
8484         s = "(null)";
8485       for(; *s; s++)
8486         consputc(*s);
8487       break;
8488     case '%':
8489       consputc('%');
8490       break;
8491     default:
8492       // Print unknown % sequence to draw attention.
8493       consputc('%');
8494       consputc(c);
8495       break;
8496     }
8497   }
8498 
8499 
8500   if(locking)
8501     release(&cons.lock);
8502 }
8503 
8504 void
8505 panic(char *s)
8506 {
8507   int i;
8508   uint pcs[10];
8509 
8510   cli();
8511   cons.locking = 0;
8512   cprintf("cpu with apicid %d: panic: ", cpu->apicid);
8513   cprintf(s);
8514   cprintf("\n");
8515   getcallerpcs(&s, pcs);
8516   for(i=0; i<10; i++)
8517     cprintf(" %p", pcs[i]);
8518   panicked = 1; // freeze other CPU
8519   for(;;)
8520     ;
8521 }
8522 
8523 
8524 
8525 
8526 
8527 
8528 
8529 
8530 
8531 
8532 
8533 
8534 
8535 
8536 
8537 
8538 
8539 
8540 
8541 
8542 
8543 
8544 
8545 
8546 
8547 
8548 
8549 
8550 #define BACKSPACE 0x100
8551 #define CRTPORT 0x3d4
8552 static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
8553 
8554 static void
8555 cgaputc(int c)
8556 {
8557   int pos;
8558 
8559   // Cursor position: col + 80*row.
8560   outb(CRTPORT, 14);
8561   pos = inb(CRTPORT+1) << 8;
8562   outb(CRTPORT, 15);
8563   pos |= inb(CRTPORT+1);
8564 
8565   if(c == '\n')
8566     pos += 80 - pos%80;
8567   else if(c == BACKSPACE){
8568     if(pos > 0) --pos;
8569   } else
8570     crt[pos++] = (c&0xff) | 0x0700;  // black on white
8571 
8572   if(pos < 0 || pos > 25*80)
8573     panic("pos under/overflow");
8574 
8575   if((pos/80) >= 24){  // Scroll up.
8576     memmove(crt, crt+80, sizeof(crt[0])*23*80);
8577     pos -= 80;
8578     memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
8579   }
8580 
8581   outb(CRTPORT, 14);
8582   outb(CRTPORT+1, pos>>8);
8583   outb(CRTPORT, 15);
8584   outb(CRTPORT+1, pos);
8585   crt[pos] = ' ' | 0x0700;
8586 }
8587 
8588 
8589 
8590 
8591 
8592 
8593 
8594 
8595 
8596 
8597 
8598 
8599 
8600 void
8601 consputc(int c)
8602 {
8603   if(panicked){
8604     cli();
8605     for(;;)
8606       ;
8607   }
8608 
8609   if(c == BACKSPACE){
8610     uartputc('\b'); uartputc(' '); uartputc('\b');
8611   } else
8612     uartputc(c);
8613   cgaputc(c);
8614 }
8615 
8616 #define INPUT_BUF 128
8617 struct {
8618   char buf[INPUT_BUF];
8619   uint r;  // Read index
8620   uint w;  // Write index
8621   uint e;  // Edit index
8622 } input;
8623 
8624 #define C(x)  ((x)-'@')  // Control-x
8625 
8626 void
8627 consoleintr(int (*getc)(void))
8628 {
8629   int c, doprocdump = 0;
8630 
8631   acquire(&cons.lock);
8632   while((c = getc()) >= 0){
8633     switch(c){
8634     case C('P'):  // Process listing.
8635       // procdump() locks cons.lock indirectly; invoke later
8636       doprocdump = 1;
8637       break;
8638     case C('U'):  // Kill line.
8639       while(input.e != input.w &&
8640             input.buf[(input.e-1) % INPUT_BUF] != '\n'){
8641         input.e--;
8642         consputc(BACKSPACE);
8643       }
8644       break;
8645     case C('H'): case '\x7f':  // Backspace
8646       if(input.e != input.w){
8647         input.e--;
8648         consputc(BACKSPACE);
8649       }
8650       break;
8651     default:
8652       if(c != 0 && input.e-input.r < INPUT_BUF){
8653         c = (c == '\r') ? '\n' : c;
8654         input.buf[input.e++ % INPUT_BUF] = c;
8655         consputc(c);
8656         if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
8657           input.w = input.e;
8658           wakeup(&input.r);
8659         }
8660       }
8661       break;
8662     }
8663   }
8664   release(&cons.lock);
8665   if(doprocdump) {
8666     procdump();  // now call procdump() wo. cons.lock held
8667   }
8668 }
8669 
8670 int
8671 consoleread(struct inode *ip, char *dst, int n)
8672 {
8673   uint target;
8674   int c;
8675 
8676   iunlock(ip);
8677   target = n;
8678   acquire(&cons.lock);
8679   while(n > 0){
8680     while(input.r == input.w){
8681       if(proc->killed){
8682         release(&cons.lock);
8683         ilock(ip);
8684         return -1;
8685       }
8686       sleep(&input.r, &cons.lock);
8687     }
8688     c = input.buf[input.r++ % INPUT_BUF];
8689     if(c == C('D')){  // EOF
8690       if(n < target){
8691         // Save ^D for next time, to make sure
8692         // caller gets a 0-byte result.
8693         input.r--;
8694       }
8695       break;
8696     }
8697     *dst++ = c;
8698     --n;
8699     if(c == '\n')
8700       break;
8701   }
8702   release(&cons.lock);
8703   ilock(ip);
8704 
8705   return target - n;
8706 }
8707 
8708 int
8709 consolewrite(struct inode *ip, char *buf, int n)
8710 {
8711   int i;
8712 
8713   iunlock(ip);
8714   acquire(&cons.lock);
8715   for(i = 0; i < n; i++)
8716     consputc(buf[i] & 0xff);
8717   release(&cons.lock);
8718   ilock(ip);
8719 
8720   return n;
8721 }
8722 
8723 void
8724 consoleinit(void)
8725 {
8726   initlock(&cons.lock, "console");
8727 
8728   devsw[CONSOLE].write = consolewrite;
8729   devsw[CONSOLE].read = consoleread;
8730   cons.locking = 1;
8731 
8732   picenable(IRQ_KBD);
8733   ioapicenable(IRQ_KBD, 0);
8734 }
8735 
8736 
8737 
8738 
8739 
8740 
8741 
8742 
8743 
8744 
8745 
8746 
8747 
8748 
8749 
