4250 struct file {
4251   enum { FD_NONE, FD_PIPE, FD_INODE } type;
4252   int ref; // reference count
4253   char readable;
4254   char writable;
4255   struct pipe *pipe;
4256   struct inode *ip;
4257   uint off;
4258 };
4259 
4260 
4261 // in-memory copy of an inode
4262 struct inode {
4263   uint dev;           // Device number
4264   uint inum;          // Inode number
4265   int ref;            // Reference count
4266   struct sleeplock lock;
4267   int flags;          // I_VALID
4268 
4269   short type;         // copy of disk inode
4270   short major;
4271   short minor;
4272   short nlink;
4273   uint size;
4274   uint addrs[NDIRECT+1];
4275 };
4276 #define I_VALID 0x2
4277 
4278 // table mapping major device number to
4279 // device functions
4280 struct devsw {
4281   int (*read)(struct inode*, char*, int);
4282   int (*write)(struct inode*, char*, int);
4283 };
4284 
4285 extern struct devsw devsw[];
4286 
4287 #define CONSOLE 1
4288 
4289 
4290 
4291 
4292 
4293 
4294 
4295 
4296 
4297 
4298 
4299 
4300 // Blank page.
4301 
4302 
4303 
4304 
4305 
4306 
4307 
4308 
4309 
4310 
4311 
4312 
4313 
4314 
4315 
4316 
4317 
4318 
4319 
4320 
4321 
4322 
4323 
4324 
4325 
4326 
4327 
4328 
4329 
4330 
4331 
4332 
4333 
4334 
4335 
4336 
4337 
4338 
4339 
4340 
4341 
4342 
4343 
4344 
4345 
4346 
4347 
4348 
4349 
