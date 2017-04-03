3150 // Physical memory allocator, intended to allocate
3151 // memory for user processes, kernel stacks, page table pages,
3152 // and pipe buffers. Allocates 4096-byte pages.
3153 
3154 #include "types.h"
3155 #include "defs.h"
3156 #include "param.h"
3157 #include "memlayout.h"
3158 #include "mmu.h"
3159 #include "spinlock.h"
3160 
3161 void freerange(void *vstart, void *vend);
3162 extern char end[]; // first address after kernel loaded from ELF file
3163 
3164 struct run {
3165   struct run *next;
3166 };
3167 
3168 struct {
3169   struct spinlock lock;
3170   int use_lock;
3171   struct run *freelist;
3172 } kmem;
3173 
3174 // Initialization happens in two phases.
3175 // 1. main() calls kinit1() while still using entrypgdir to place just
3176 // the pages mapped by entrypgdir on free list.
3177 // 2. main() calls kinit2() with the rest of the physical pages
3178 // after installing a full page table that maps them on all cores.
3179 void
3180 kinit1(void *vstart, void *vend)
3181 {
3182   initlock(&kmem.lock, "kmem");
3183   kmem.use_lock = 0;
3184   freerange(vstart, vend);
3185 }
3186 
3187 void
3188 kinit2(void *vstart, void *vend)
3189 {
3190   freerange(vstart, vend);
3191   kmem.use_lock = 1;
3192 }
3193 
3194 
3195 
3196 
3197 
3198 
3199 
3200 void
3201 freerange(void *vstart, void *vend)
3202 {
3203   char *p;
3204   p = (char*)PGROUNDUP((uint)vstart);
3205   for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
3206     kfree(p);
3207 }
3208 
3209 
3210 // Free the page of physical memory pointed at by v,
3211 // which normally should have been returned by a
3212 // call to kalloc().  (The exception is when
3213 // initializing the allocator; see kinit above.)
3214 void
3215 kfree(char *v)
3216 {
3217   struct run *r;
3218 
3219   if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
3220     panic("kfree");
3221 
3222   // Fill with junk to catch dangling refs.
3223   memset(v, 1, PGSIZE);
3224 
3225   if(kmem.use_lock)
3226     acquire(&kmem.lock);
3227   r = (struct run*)v;
3228   r->next = kmem.freelist;
3229   kmem.freelist = r;
3230   if(kmem.use_lock)
3231     release(&kmem.lock);
3232 }
3233 
3234 // Allocate one 4096-byte page of physical memory.
3235 // Returns a pointer that the kernel can use.
3236 // Returns 0 if the memory cannot be allocated.
3237 char*
3238 kalloc(void)
3239 {
3240   struct run *r;
3241 
3242   if(kmem.use_lock)
3243     acquire(&kmem.lock);
3244   r = kmem.freelist;
3245   if(r)
3246     kmem.freelist = r->next;
3247   if(kmem.use_lock)
3248     release(&kmem.lock);
3249   return (char*)r;
3250 }
3251 
3252 
3253 
3254 
3255 
3256 
3257 
3258 
3259 
3260 
3261 
3262 
3263 
3264 
3265 
3266 
3267 
3268 
3269 
3270 
3271 
3272 
3273 
3274 
3275 
3276 
3277 
3278 
3279 
3280 
3281 
3282 
3283 
3284 
3285 
3286 
3287 
3288 
3289 
3290 
3291 
3292 
3293 
3294 
3295 
3296 
3297 
3298 
3299 
