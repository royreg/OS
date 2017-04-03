5100 // File system implementation.  Five layers:
5101 //   + Blocks: allocator for raw disk blocks.
5102 //   + Log: crash recovery for multi-step updates.
5103 //   + Files: inode allocator, reading, writing, metadata.
5104 //   + Directories: inode with special contents (list of other inodes!)
5105 //   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
5106 //
5107 // This file contains the low-level file system manipulation
5108 // routines.  The (higher-level) system call implementations
5109 // are in sysfile.c.
5110 
5111 #include "types.h"
5112 #include "defs.h"
5113 #include "param.h"
5114 #include "stat.h"
5115 #include "mmu.h"
5116 #include "proc.h"
5117 #include "spinlock.h"
5118 #include "sleeplock.h"
5119 #include "fs.h"
5120 #include "buf.h"
5121 #include "file.h"
5122 
5123 #define min(a, b) ((a) < (b) ? (a) : (b))
5124 static void itrunc(struct inode*);
5125 // there should be one superblock per disk device, but we run with
5126 // only one device
5127 struct superblock sb;
5128 
5129 // Read the super block.
5130 void
5131 readsb(int dev, struct superblock *sb)
5132 {
5133   struct buf *bp;
5134 
5135   bp = bread(dev, 1);
5136   memmove(sb, bp->data, sizeof(*sb));
5137   brelse(bp);
5138 }
5139 
5140 
5141 
5142 
5143 
5144 
5145 
5146 
5147 
5148 
5149 
5150 // Zero a block.
5151 static void
5152 bzero(int dev, int bno)
5153 {
5154   struct buf *bp;
5155 
5156   bp = bread(dev, bno);
5157   memset(bp->data, 0, BSIZE);
5158   log_write(bp);
5159   brelse(bp);
5160 }
5161 
5162 // Blocks.
5163 
5164 // Allocate a zeroed disk block.
5165 static uint
5166 balloc(uint dev)
5167 {
5168   int b, bi, m;
5169   struct buf *bp;
5170 
5171   bp = 0;
5172   for(b = 0; b < sb.size; b += BPB){
5173     bp = bread(dev, BBLOCK(b, sb));
5174     for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
5175       m = 1 << (bi % 8);
5176       if((bp->data[bi/8] & m) == 0){  // Is block free?
5177         bp->data[bi/8] |= m;  // Mark block in use.
5178         log_write(bp);
5179         brelse(bp);
5180         bzero(dev, b + bi);
5181         return b + bi;
5182       }
5183     }
5184     brelse(bp);
5185   }
5186   panic("balloc: out of blocks");
5187 }
5188 
5189 
5190 
5191 
5192 
5193 
5194 
5195 
5196 
5197 
5198 
5199 
5200 // Free a disk block.
5201 static void
5202 bfree(int dev, uint b)
5203 {
5204   struct buf *bp;
5205   int bi, m;
5206 
5207   readsb(dev, &sb);
5208   bp = bread(dev, BBLOCK(b, sb));
5209   bi = b % BPB;
5210   m = 1 << (bi % 8);
5211   if((bp->data[bi/8] & m) == 0)
5212     panic("freeing free block");
5213   bp->data[bi/8] &= ~m;
5214   log_write(bp);
5215   brelse(bp);
5216 }
5217 
5218 // Inodes.
5219 //
5220 // An inode describes a single unnamed file.
5221 // The inode disk structure holds metadata: the file's type,
5222 // its size, the number of links referring to it, and the
5223 // list of blocks holding the file's content.
5224 //
5225 // The inodes are laid out sequentially on disk at
5226 // sb.startinode. Each inode has a number, indicating its
5227 // position on the disk.
5228 //
5229 // The kernel keeps a cache of in-use inodes in memory
5230 // to provide a place for synchronizing access
5231 // to inodes used by multiple processes. The cached
5232 // inodes include book-keeping information that is
5233 // not stored on disk: ip->ref and ip->flags.
5234 //
5235 // An inode and its in-memory represtative go through a
5236 // sequence of states before they can be used by the
5237 // rest of the file system code.
5238 //
5239 // * Allocation: an inode is allocated if its type (on disk)
5240 //   is non-zero. ialloc() allocates, iput() frees if
5241 //   the link count has fallen to zero.
5242 //
5243 // * Referencing in cache: an entry in the inode cache
5244 //   is free if ip->ref is zero. Otherwise ip->ref tracks
5245 //   the number of in-memory pointers to the entry (open
5246 //   files and current directories). iget() to find or
5247 //   create a cache entry and increment its ref, iput()
5248 //   to decrement ref.
5249 //
5250 // * Valid: the information (type, size, &c) in an inode
5251 //   cache entry is only correct when the I_VALID bit
5252 //   is set in ip->flags. ilock() reads the inode from
5253 //   the disk and sets I_VALID, while iput() clears
5254 //   I_VALID if ip->ref has fallen to zero.
5255 //
5256 // * Locked: file system code may only examine and modify
5257 //   the information in an inode and its content if it
5258 //   has first locked the inode.
5259 //
5260 // Thus a typical sequence is:
5261 //   ip = iget(dev, inum)
5262 //   ilock(ip)
5263 //   ... examine and modify ip->xxx ...
5264 //   iunlock(ip)
5265 //   iput(ip)
5266 //
5267 // ilock() is separate from iget() so that system calls can
5268 // get a long-term reference to an inode (as for an open file)
5269 // and only lock it for short periods (e.g., in read()).
5270 // The separation also helps avoid deadlock and races during
5271 // pathname lookup. iget() increments ip->ref so that the inode
5272 // stays cached and pointers to it remain valid.
5273 //
5274 // Many internal file system functions expect the caller to
5275 // have locked the inodes involved; this lets callers create
5276 // multi-step atomic operations.
5277 
5278 struct {
5279   struct spinlock lock;
5280   struct inode inode[NINODE];
5281 } icache;
5282 
5283 void
5284 iinit(int dev)
5285 {
5286   int i = 0;
5287 
5288   initlock(&icache.lock, "icache");
5289   for(i = 0; i < NINODE; i++) {
5290     initsleeplock(&icache.inode[i].lock, "inode");
5291   }
5292 
5293   readsb(dev, &sb);
5294   cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d\
5295  inodestart %d bmap start %d\n", sb.size, sb.nblocks,
5296           sb.ninodes, sb.nlog, sb.logstart, sb.inodestart,
5297           sb.bmapstart);
5298 }
5299 
5300 static struct inode* iget(uint dev, uint inum);
5301 
5302 
5303 
5304 
5305 
5306 
5307 
5308 
5309 
5310 
5311 
5312 
5313 
5314 
5315 
5316 
5317 
5318 
5319 
5320 
5321 
5322 
5323 
5324 
5325 
5326 
5327 
5328 
5329 
5330 
5331 
5332 
5333 
5334 
5335 
5336 
5337 
5338 
5339 
5340 
5341 
5342 
5343 
5344 
5345 
5346 
5347 
5348 
5349 
5350 // Allocate a new inode with the given type on device dev.
5351 // A free inode has a type of zero.
5352 struct inode*
5353 ialloc(uint dev, short type)
5354 {
5355   int inum;
5356   struct buf *bp;
5357   struct dinode *dip;
5358 
5359   for(inum = 1; inum < sb.ninodes; inum++){
5360     bp = bread(dev, IBLOCK(inum, sb));
5361     dip = (struct dinode*)bp->data + inum%IPB;
5362     if(dip->type == 0){  // a free inode
5363       memset(dip, 0, sizeof(*dip));
5364       dip->type = type;
5365       log_write(bp);   // mark it allocated on the disk
5366       brelse(bp);
5367       return iget(dev, inum);
5368     }
5369     brelse(bp);
5370   }
5371   panic("ialloc: no inodes");
5372 }
5373 
5374 // Copy a modified in-memory inode to disk.
5375 void
5376 iupdate(struct inode *ip)
5377 {
5378   struct buf *bp;
5379   struct dinode *dip;
5380 
5381   bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5382   dip = (struct dinode*)bp->data + ip->inum%IPB;
5383   dip->type = ip->type;
5384   dip->major = ip->major;
5385   dip->minor = ip->minor;
5386   dip->nlink = ip->nlink;
5387   dip->size = ip->size;
5388   memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
5389   log_write(bp);
5390   brelse(bp);
5391 }
5392 
5393 
5394 
5395 
5396 
5397 
5398 
5399 
5400 // Find the inode with number inum on device dev
5401 // and return the in-memory copy. Does not lock
5402 // the inode and does not read it from disk.
5403 static struct inode*
5404 iget(uint dev, uint inum)
5405 {
5406   struct inode *ip, *empty;
5407 
5408   acquire(&icache.lock);
5409 
5410   // Is the inode already cached?
5411   empty = 0;
5412   for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
5413     if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
5414       ip->ref++;
5415       release(&icache.lock);
5416       return ip;
5417     }
5418     if(empty == 0 && ip->ref == 0)    // Remember empty slot.
5419       empty = ip;
5420   }
5421 
5422   // Recycle an inode cache entry.
5423   if(empty == 0)
5424     panic("iget: no inodes");
5425 
5426   ip = empty;
5427   ip->dev = dev;
5428   ip->inum = inum;
5429   ip->ref = 1;
5430   ip->flags = 0;
5431   release(&icache.lock);
5432 
5433   return ip;
5434 }
5435 
5436 // Increment reference count for ip.
5437 // Returns ip to enable ip = idup(ip1) idiom.
5438 struct inode*
5439 idup(struct inode *ip)
5440 {
5441   acquire(&icache.lock);
5442   ip->ref++;
5443   release(&icache.lock);
5444   return ip;
5445 }
5446 
5447 
5448 
5449 
5450 // Lock the given inode.
5451 // Reads the inode from disk if necessary.
5452 void
5453 ilock(struct inode *ip)
5454 {
5455   struct buf *bp;
5456   struct dinode *dip;
5457 
5458   if(ip == 0 || ip->ref < 1)
5459     panic("ilock");
5460 
5461   acquiresleep(&ip->lock);
5462 
5463   if(!(ip->flags & I_VALID)){
5464     bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5465     dip = (struct dinode*)bp->data + ip->inum%IPB;
5466     ip->type = dip->type;
5467     ip->major = dip->major;
5468     ip->minor = dip->minor;
5469     ip->nlink = dip->nlink;
5470     ip->size = dip->size;
5471     memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
5472     brelse(bp);
5473     ip->flags |= I_VALID;
5474     if(ip->type == 0)
5475       panic("ilock: no type");
5476   }
5477 }
5478 
5479 // Unlock the given inode.
5480 void
5481 iunlock(struct inode *ip)
5482 {
5483   if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
5484     panic("iunlock");
5485 
5486   releasesleep(&ip->lock);
5487 }
5488 
5489 
5490 
5491 
5492 
5493 
5494 
5495 
5496 
5497 
5498 
5499 
5500 // Drop a reference to an in-memory inode.
5501 // If that was the last reference, the inode cache entry can
5502 // be recycled.
5503 // If that was the last reference and the inode has no links
5504 // to it, free the inode (and its content) on disk.
5505 // All calls to iput() must be inside a transaction in
5506 // case it has to free the inode.
5507 void
5508 iput(struct inode *ip)
5509 {
5510   acquire(&icache.lock);
5511   if(ip->ref == 1 && (ip->flags & I_VALID) && ip->nlink == 0){
5512     // inode has no links and no other references: truncate and free.
5513     release(&icache.lock);
5514     itrunc(ip);
5515     ip->type = 0;
5516     iupdate(ip);
5517     acquire(&icache.lock);
5518     ip->flags = 0;
5519   }
5520   ip->ref--;
5521   release(&icache.lock);
5522 }
5523 
5524 // Common idiom: unlock, then put.
5525 void
5526 iunlockput(struct inode *ip)
5527 {
5528   iunlock(ip);
5529   iput(ip);
5530 }
5531 
5532 
5533 
5534 
5535 
5536 
5537 
5538 
5539 
5540 
5541 
5542 
5543 
5544 
5545 
5546 
5547 
5548 
5549 
5550 // Inode content
5551 //
5552 // The content (data) associated with each inode is stored
5553 // in blocks on the disk. The first NDIRECT block numbers
5554 // are listed in ip->addrs[].  The next NINDIRECT blocks are
5555 // listed in block ip->addrs[NDIRECT].
5556 
5557 // Return the disk block address of the nth block in inode ip.
5558 // If there is no such block, bmap allocates one.
5559 static uint
5560 bmap(struct inode *ip, uint bn)
5561 {
5562   uint addr, *a;
5563   struct buf *bp;
5564 
5565   if(bn < NDIRECT){
5566     if((addr = ip->addrs[bn]) == 0)
5567       ip->addrs[bn] = addr = balloc(ip->dev);
5568     return addr;
5569   }
5570   bn -= NDIRECT;
5571 
5572   if(bn < NINDIRECT){
5573     // Load indirect block, allocating if necessary.
5574     if((addr = ip->addrs[NDIRECT]) == 0)
5575       ip->addrs[NDIRECT] = addr = balloc(ip->dev);
5576     bp = bread(ip->dev, addr);
5577     a = (uint*)bp->data;
5578     if((addr = a[bn]) == 0){
5579       a[bn] = addr = balloc(ip->dev);
5580       log_write(bp);
5581     }
5582     brelse(bp);
5583     return addr;
5584   }
5585 
5586   panic("bmap: out of range");
5587 }
5588 
5589 
5590 
5591 
5592 
5593 
5594 
5595 
5596 
5597 
5598 
5599 
5600 // Truncate inode (discard contents).
5601 // Only called when the inode has no links
5602 // to it (no directory entries referring to it)
5603 // and has no in-memory reference to it (is
5604 // not an open file or current directory).
5605 static void
5606 itrunc(struct inode *ip)
5607 {
5608   int i, j;
5609   struct buf *bp;
5610   uint *a;
5611 
5612   for(i = 0; i < NDIRECT; i++){
5613     if(ip->addrs[i]){
5614       bfree(ip->dev, ip->addrs[i]);
5615       ip->addrs[i] = 0;
5616     }
5617   }
5618 
5619   if(ip->addrs[NDIRECT]){
5620     bp = bread(ip->dev, ip->addrs[NDIRECT]);
5621     a = (uint*)bp->data;
5622     for(j = 0; j < NINDIRECT; j++){
5623       if(a[j])
5624         bfree(ip->dev, a[j]);
5625     }
5626     brelse(bp);
5627     bfree(ip->dev, ip->addrs[NDIRECT]);
5628     ip->addrs[NDIRECT] = 0;
5629   }
5630 
5631   ip->size = 0;
5632   iupdate(ip);
5633 }
5634 
5635 // Copy stat information from inode.
5636 void
5637 stati(struct inode *ip, struct stat *st)
5638 {
5639   st->dev = ip->dev;
5640   st->ino = ip->inum;
5641   st->type = ip->type;
5642   st->nlink = ip->nlink;
5643   st->size = ip->size;
5644 }
5645 
5646 
5647 
5648 
5649 
5650 // Read data from inode.
5651 int
5652 readi(struct inode *ip, char *dst, uint off, uint n)
5653 {
5654   uint tot, m;
5655   struct buf *bp;
5656 
5657   if(ip->type == T_DEV){
5658     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
5659       return -1;
5660     return devsw[ip->major].read(ip, dst, n);
5661   }
5662 
5663   if(off > ip->size || off + n < off)
5664     return -1;
5665   if(off + n > ip->size)
5666     n = ip->size - off;
5667 
5668   for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
5669     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5670     m = min(n - tot, BSIZE - off%BSIZE);
5671     /*
5672     cprintf("data off %d:\n", off);
5673     for (int j = 0; j < min(m, 10); j++) {
5674       cprintf("%x ", bp->data[off%BSIZE+j]);
5675     }
5676     cprintf("\n");
5677     */
5678     memmove(dst, bp->data + off%BSIZE, m);
5679     brelse(bp);
5680   }
5681   return n;
5682 }
5683 
5684 
5685 
5686 
5687 
5688 
5689 
5690 
5691 
5692 
5693 
5694 
5695 
5696 
5697 
5698 
5699 
5700 // Write data to inode.
5701 int
5702 writei(struct inode *ip, char *src, uint off, uint n)
5703 {
5704   uint tot, m;
5705   struct buf *bp;
5706 
5707   if(ip->type == T_DEV){
5708     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
5709       return -1;
5710     return devsw[ip->major].write(ip, src, n);
5711   }
5712 
5713   if(off > ip->size || off + n < off)
5714     return -1;
5715   if(off + n > MAXFILE*BSIZE)
5716     return -1;
5717 
5718   for(tot=0; tot<n; tot+=m, off+=m, src+=m){
5719     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5720     m = min(n - tot, BSIZE - off%BSIZE);
5721     memmove(bp->data + off%BSIZE, src, m);
5722     log_write(bp);
5723     brelse(bp);
5724   }
5725 
5726   if(n > 0 && off > ip->size){
5727     ip->size = off;
5728     iupdate(ip);
5729   }
5730   return n;
5731 }
5732 
5733 
5734 
5735 
5736 
5737 
5738 
5739 
5740 
5741 
5742 
5743 
5744 
5745 
5746 
5747 
5748 
5749 
5750 // Directories
5751 
5752 int
5753 namecmp(const char *s, const char *t)
5754 {
5755   return strncmp(s, t, DIRSIZ);
5756 }
5757 
5758 // Look for a directory entry in a directory.
5759 // If found, set *poff to byte offset of entry.
5760 struct inode*
5761 dirlookup(struct inode *dp, char *name, uint *poff)
5762 {
5763   uint off, inum;
5764   struct dirent de;
5765 
5766   if(dp->type != T_DIR)
5767     panic("dirlookup not DIR");
5768 
5769   for(off = 0; off < dp->size; off += sizeof(de)){
5770     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5771       panic("dirlink read");
5772     if(de.inum == 0)
5773       continue;
5774     if(namecmp(name, de.name) == 0){
5775       // entry matches path element
5776       if(poff)
5777         *poff = off;
5778       inum = de.inum;
5779       return iget(dp->dev, inum);
5780     }
5781   }
5782 
5783   return 0;
5784 }
5785 
5786 
5787 
5788 
5789 
5790 
5791 
5792 
5793 
5794 
5795 
5796 
5797 
5798 
5799 
5800 // Write a new directory entry (name, inum) into the directory dp.
5801 int
5802 dirlink(struct inode *dp, char *name, uint inum)
5803 {
5804   int off;
5805   struct dirent de;
5806   struct inode *ip;
5807 
5808   // Check that name is not present.
5809   if((ip = dirlookup(dp, name, 0)) != 0){
5810     iput(ip);
5811     return -1;
5812   }
5813 
5814   // Look for an empty dirent.
5815   for(off = 0; off < dp->size; off += sizeof(de)){
5816     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5817       panic("dirlink read");
5818     if(de.inum == 0)
5819       break;
5820   }
5821 
5822   strncpy(de.name, name, DIRSIZ);
5823   de.inum = inum;
5824   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5825     panic("dirlink");
5826 
5827   return 0;
5828 }
5829 
5830 
5831 
5832 
5833 
5834 
5835 
5836 
5837 
5838 
5839 
5840 
5841 
5842 
5843 
5844 
5845 
5846 
5847 
5848 
5849 
5850 // Paths
5851 
5852 // Copy the next path element from path into name.
5853 // Return a pointer to the element following the copied one.
5854 // The returned path has no leading slashes,
5855 // so the caller can check *path=='\0' to see if the name is the last one.
5856 // If no name to remove, return 0.
5857 //
5858 // Examples:
5859 //   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
5860 //   skipelem("///a//bb", name) = "bb", setting name = "a"
5861 //   skipelem("a", name) = "", setting name = "a"
5862 //   skipelem("", name) = skipelem("////", name) = 0
5863 //
5864 static char*
5865 skipelem(char *path, char *name)
5866 {
5867   char *s;
5868   int len;
5869 
5870   while(*path == '/')
5871     path++;
5872   if(*path == 0)
5873     return 0;
5874   s = path;
5875   while(*path != '/' && *path != 0)
5876     path++;
5877   len = path - s;
5878   if(len >= DIRSIZ)
5879     memmove(name, s, DIRSIZ);
5880   else {
5881     memmove(name, s, len);
5882     name[len] = 0;
5883   }
5884   while(*path == '/')
5885     path++;
5886   return path;
5887 }
5888 
5889 
5890 
5891 
5892 
5893 
5894 
5895 
5896 
5897 
5898 
5899 
5900 // Look up and return the inode for a path name.
5901 // If parent != 0, return the inode for the parent and copy the final
5902 // path element into name, which must have room for DIRSIZ bytes.
5903 // Must be called inside a transaction since it calls iput().
5904 static struct inode*
5905 namex(char *path, int nameiparent, char *name)
5906 {
5907   struct inode *ip, *next;
5908 
5909   if(*path == '/')
5910     ip = iget(ROOTDEV, ROOTINO);
5911   else
5912     ip = idup(proc->cwd);
5913 
5914   while((path = skipelem(path, name)) != 0){
5915     ilock(ip);
5916     if(ip->type != T_DIR){
5917       iunlockput(ip);
5918       return 0;
5919     }
5920     if(nameiparent && *path == '\0'){
5921       // Stop one level early.
5922       iunlock(ip);
5923       return ip;
5924     }
5925     if((next = dirlookup(ip, name, 0)) == 0){
5926       iunlockput(ip);
5927       return 0;
5928     }
5929     iunlockput(ip);
5930     ip = next;
5931   }
5932   if(nameiparent){
5933     iput(ip);
5934     return 0;
5935   }
5936   return ip;
5937 }
5938 
5939 struct inode*
5940 namei(char *path)
5941 {
5942   char name[DIRSIZ];
5943   return namex(path, 0, name);
5944 }
5945 
5946 
5947 
5948 
5949 
5950 struct inode*
5951 nameiparent(char *path, char *name)
5952 {
5953   return namex(path, 1, name);
5954 }
5955 
5956 
5957 
5958 
5959 
5960 
5961 
5962 
5963 
5964 
5965 
5966 
5967 
5968 
5969 
5970 
5971 
5972 
5973 
5974 
5975 
5976 
5977 
5978 
5979 
5980 
5981 
5982 
5983 
5984 
5985 
5986 
5987 
5988 
5989 
5990 
5991 
5992 
5993 
5994 
5995 
5996 
5997 
5998 
5999 
