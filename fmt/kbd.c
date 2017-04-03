8300 #include "types.h"
8301 #include "x86.h"
8302 #include "defs.h"
8303 #include "kbd.h"
8304 
8305 int
8306 kbdgetc(void)
8307 {
8308   static uint shift;
8309   static uchar *charcode[4] = {
8310     normalmap, shiftmap, ctlmap, ctlmap
8311   };
8312   uint st, data, c;
8313 
8314   st = inb(KBSTATP);
8315   if((st & KBS_DIB) == 0)
8316     return -1;
8317   data = inb(KBDATAP);
8318 
8319   if(data == 0xE0){
8320     shift |= E0ESC;
8321     return 0;
8322   } else if(data & 0x80){
8323     // Key released
8324     data = (shift & E0ESC ? data : data & 0x7F);
8325     shift &= ~(shiftcode[data] | E0ESC);
8326     return 0;
8327   } else if(shift & E0ESC){
8328     // Last character was an E0 escape; or with 0x80
8329     data |= 0x80;
8330     shift &= ~E0ESC;
8331   }
8332 
8333   shift |= shiftcode[data];
8334   shift ^= togglecode[data];
8335   c = charcode[shift & (CTL | SHIFT)][data];
8336   if(shift & CAPSLOCK){
8337     if('a' <= c && c <= 'z')
8338       c += 'A' - 'a';
8339     else if('A' <= c && c <= 'Z')
8340       c += 'a' - 'A';
8341   }
8342   return c;
8343 }
8344 
8345 void
8346 kbdintr(void)
8347 {
8348   consoleintr(kbdgetc);
8349 }
