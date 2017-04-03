9050 // Shell.
9051 
9052 #include "types.h"
9053 #include "user.h"
9054 #include "fcntl.h"
9055 
9056 
9057 
9058 
9059 // Parsed command representation
9060 #define EXEC  1
9061 #define REDIR 2
9062 #define PIPE  3
9063 #define LIST  4
9064 #define BACK  5
9065 
9066 #define MAXARGS 10
9067 
9068 struct cmd {
9069   int type;
9070 };
9071 
9072 struct execcmd {
9073   int type;
9074   char *argv[MAXARGS];
9075   char *eargv[MAXARGS];
9076 };
9077 
9078 struct redircmd {
9079   int type;
9080   struct cmd *cmd;
9081   char *file;
9082   char *efile;
9083   int mode;
9084   int fd;
9085 };
9086 
9087 struct pipecmd {
9088   int type;
9089   struct cmd *left;
9090   struct cmd *right;
9091 };
9092 
9093 struct listcmd {
9094   int type;
9095   struct cmd *left;
9096   struct cmd *right;
9097 };
9098 
9099 
9100 struct backcmd {
9101   int type;
9102   struct cmd *cmd;
9103 };
9104 
9105 int fork1(void);  // Fork but panics on failure.
9106 void panic(char*);
9107 struct cmd *parsecmd(char*);
9108 
9109 
9110 //initilaize a string with 0;
9111 void strClear(char s[],int len){
9112   int i=0;
9113   if(len){
9114     while(i<len){
9115       s[i]=0;
9116       i++;
9117     }
9118   }
9119 }
9120 
9121 
9122 
9123 //check and update cmd path with the global environment path
9124 void checkPath(struct execcmd *execCmd){
9125   int fd=open(execCmd->argv[0],O_RDWR);
9126   if(fd>0){
9127     return;
9128   }
9129 
9130   fd=open("/path",O_RDWR);
9131   char tempPath[50];
9132   while(1){
9133     int firstLetter=1;
9134     int ind=0;
9135     strClear(tempPath,50);
9136     int stat=read(fd,tempPath,1);
9137     //printf(2,"stat is : %d\n",stat);
9138     //printf(2,"the first LEttet is %d\n",tempPath[0]);
9139     if(stat<=0||tempPath[0]=='\n'){
9140      // printf(2,"%s\n","end of file");
9141       break;
9142     }
9143     //printf(2,"the path is %s\n",tempPath);
9144 
9145     while(1){
9146       if(firstLetter==1) { //dosen't need to read again'
9147         firstLetter=0;
9148 
9149 
9150         }
9151       else{
9152         read(fd,tempPath+ind,1);
9153       }
9154 
9155       if(tempPath[ind]==':'){
9156         //printf(2,"read : %s\n",tempPath);
9157         break;
9158       }
9159 
9160       else
9161         ind++;
9162     }//end of while
9163 
9164     strcpy(tempPath+ind,execCmd->argv[0]);
9165     //printf(2,"the path is %s\n",tempPath);
9166     int tempfd=open(tempPath,O_RDONLY);
9167     if(tempfd>0){
9168       strcpy(execCmd->argv[0],tempPath);
9169       close(fd);
9170       close(tempfd);
9171       return;
9172     }
9173   }//end of while
9174   close(fd);
9175 
9176 }
9177 
9178 
9179 
9180 // Execute cmd.  Never returns.
9181 void
9182 runcmd(struct cmd *cmd)
9183 {
9184   int p[2];
9185   struct backcmd *bcmd;
9186   struct execcmd *ecmd;
9187   struct listcmd *lcmd;
9188   struct pipecmd *pcmd;
9189   struct redircmd *rcmd;
9190 
9191   if(cmd == 0)
9192     exit(0);
9193 
9194   switch(cmd->type){
9195   default:
9196     panic("runcmd");
9197 
9198 
9199 
9200   case EXEC:
9201     ecmd = (struct execcmd*)cmd;
9202     if(ecmd->argv[0] == 0)
9203       exit(0);
9204     checkPath(ecmd);
9205     //printf(2,"%s",ecmd->argv[0]);
9206     exec(ecmd->argv[0], ecmd->argv);
9207     printf(2, "exec %s failed\n", ecmd->argv[0]);
9208     break;
9209 
9210   case REDIR:
9211     rcmd = (struct redircmd*)cmd;
9212     close(rcmd->fd);
9213     if(open(rcmd->file, rcmd->mode) < 0){
9214       printf(2, "open %s failed\n", rcmd->file);
9215       exit(0);
9216     }
9217     runcmd(rcmd->cmd);
9218     break;
9219 
9220   case LIST:
9221     lcmd = (struct listcmd*)cmd;
9222     if(fork1() == 0)
9223       runcmd(lcmd->left);
9224     wait(0);
9225     runcmd(lcmd->right);
9226     break;
9227 
9228   case PIPE:
9229     pcmd = (struct pipecmd*)cmd;
9230     if(pipe(p) < 0)
9231       panic("pipe");
9232     if(fork1() == 0){
9233       close(1);
9234       dup(p[1]);
9235       close(p[0]);
9236       close(p[1]);
9237       runcmd(pcmd->left);
9238     }
9239     if(fork1() == 0){
9240       close(0);
9241       dup(p[0]);
9242       close(p[0]);
9243       close(p[1]);
9244       runcmd(pcmd->right);
9245     }
9246     close(p[0]);
9247     close(p[1]);
9248     wait(0);
9249     wait(0);
9250     break;
9251 
9252   case BACK:
9253     bcmd = (struct backcmd*)cmd;
9254     if(fork1() == 0)
9255       runcmd(bcmd->cmd);
9256     break;
9257   }
9258   exit(0);
9259 }
9260 
9261 int
9262 getcmd(char *buf, int nbuf)
9263 {
9264   printf(2, "$ ");
9265   memset(buf, 0, nbuf);
9266   gets(buf, nbuf);
9267   if(buf[0] == 0) // EOF
9268     return -1;
9269   return 0;
9270 }
9271 
9272 int
9273 main(void)
9274 {
9275   static char buf[100];
9276   int fd;
9277 
9278   // Ensure that three file descriptors are open.
9279   while((fd = open("console", O_RDWR)) >= 0){
9280     if(fd >= 3){
9281       close(fd);
9282       break;
9283     }
9284   }
9285 
9286   // Read and run input commands.
9287   while(getcmd(buf, sizeof(buf)) >= 0){
9288     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
9289       // Chdir must be called by the parent, not the child.
9290       buf[strlen(buf)-1] = 0;  // chop \n
9291       if(chdir(buf+3) < 0)
9292         printf(2, "cannot cd %s\n", buf+3);
9293       continue;
9294     }
9295     if(fork1() == 0)
9296       runcmd(parsecmd(buf));
9297     wait(0);
9298   }
9299   exit(0);
9300 }
9301 
9302 void
9303 panic(char *s)
9304 {
9305   printf(2, "%s\n", s);
9306   exit(0);
9307 }
9308 
9309 int
9310 fork1(void)
9311 {
9312   int pid;
9313 
9314   pid = fork();
9315   if(pid == -1)
9316     panic("fork");
9317   return pid;
9318 }
9319 
9320 
9321 
9322 
9323 
9324 
9325 
9326 
9327 
9328 
9329 
9330 
9331 
9332 
9333 
9334 
9335 
9336 
9337 
9338 
9339 
9340 
9341 
9342 
9343 
9344 
9345 
9346 
9347 
9348 
9349 
9350 // Constructors
9351 
9352 struct cmd*
9353 execcmd(void)
9354 {
9355   struct execcmd *cmd;
9356 
9357   cmd = malloc(sizeof(*cmd));
9358   memset(cmd, 0, sizeof(*cmd));
9359   cmd->type = EXEC;
9360   return (struct cmd*)cmd;
9361 }
9362 
9363 struct cmd*
9364 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
9365 {
9366   struct redircmd *cmd;
9367 
9368   cmd = malloc(sizeof(*cmd));
9369   memset(cmd, 0, sizeof(*cmd));
9370   cmd->type = REDIR;
9371   cmd->cmd = subcmd;
9372   cmd->file = file;
9373   cmd->efile = efile;
9374   cmd->mode = mode;
9375   cmd->fd = fd;
9376   return (struct cmd*)cmd;
9377 }
9378 
9379 struct cmd*
9380 pipecmd(struct cmd *left, struct cmd *right)
9381 {
9382   struct pipecmd *cmd;
9383 
9384   cmd = malloc(sizeof(*cmd));
9385   memset(cmd, 0, sizeof(*cmd));
9386   cmd->type = PIPE;
9387   cmd->left = left;
9388   cmd->right = right;
9389   return (struct cmd*)cmd;
9390 }
9391 
9392 
9393 
9394 
9395 
9396 
9397 
9398 
9399 
9400 struct cmd*
9401 listcmd(struct cmd *left, struct cmd *right)
9402 {
9403   struct listcmd *cmd;
9404 
9405   cmd = malloc(sizeof(*cmd));
9406   memset(cmd, 0, sizeof(*cmd));
9407   cmd->type = LIST;
9408   cmd->left = left;
9409   cmd->right = right;
9410   return (struct cmd*)cmd;
9411 }
9412 
9413 struct cmd*
9414 backcmd(struct cmd *subcmd)
9415 {
9416   struct backcmd *cmd;
9417 
9418   cmd = malloc(sizeof(*cmd));
9419   memset(cmd, 0, sizeof(*cmd));
9420   cmd->type = BACK;
9421   cmd->cmd = subcmd;
9422   return (struct cmd*)cmd;
9423 }
9424 
9425 
9426 
9427 
9428 
9429 
9430 
9431 
9432 
9433 
9434 
9435 
9436 
9437 
9438 
9439 
9440 
9441 
9442 
9443 
9444 
9445 
9446 
9447 
9448 
9449 
9450 // Parsing
9451 
9452 char whitespace[] = " \t\r\n\v";
9453 char symbols[] = "<|>&;()";
9454 
9455 int
9456 gettoken(char **ps, char *es, char **q, char **eq)
9457 {
9458   char *s;
9459   int ret;
9460 
9461   s = *ps;
9462   while(s < es && strchr(whitespace, *s))
9463     s++;
9464   if(q)
9465     *q = s;
9466   ret = *s;
9467   switch(*s){
9468   case 0:
9469     break;
9470   case '|':
9471   case '(':
9472   case ')':
9473   case ';':
9474   case '&':
9475   case '<':
9476     s++;
9477     break;
9478   case '>':
9479     s++;
9480     if(*s == '>'){
9481       ret = '+';
9482       s++;
9483     }
9484     break;
9485   default:
9486     ret = 'a';
9487     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
9488       s++;
9489     break;
9490   }
9491   if(eq)
9492     *eq = s;
9493 
9494   while(s < es && strchr(whitespace, *s))
9495     s++;
9496   *ps = s;
9497   return ret;
9498 }
9499 
9500 int
9501 peek(char **ps, char *es, char *toks)
9502 {
9503   char *s;
9504 
9505   s = *ps;
9506   while(s < es && strchr(whitespace, *s))
9507     s++;
9508   *ps = s;
9509   return *s && strchr(toks, *s);
9510 }
9511 
9512 struct cmd *parseline(char**, char*);
9513 struct cmd *parsepipe(char**, char*);
9514 struct cmd *parseexec(char**, char*);
9515 struct cmd *nulterminate(struct cmd*);
9516 
9517 struct cmd*
9518 parsecmd(char *s)
9519 {
9520   char *es;
9521   struct cmd *cmd;
9522 
9523   es = s + strlen(s);
9524   cmd = parseline(&s, es);
9525   peek(&s, es, "");
9526   if(s != es){
9527     printf(2, "leftovers: %s\n", s);
9528     panic("syntax");
9529   }
9530   nulterminate(cmd);
9531   return cmd;
9532 }
9533 
9534 struct cmd*
9535 parseline(char **ps, char *es)
9536 {
9537   struct cmd *cmd;
9538 
9539   cmd = parsepipe(ps, es);
9540   while(peek(ps, es, "&")){
9541     gettoken(ps, es, 0, 0);
9542     cmd = backcmd(cmd);
9543   }
9544   if(peek(ps, es, ";")){
9545     gettoken(ps, es, 0, 0);
9546     cmd = listcmd(cmd, parseline(ps, es));
9547   }
9548   return cmd;
9549 }
9550 struct cmd*
9551 parsepipe(char **ps, char *es)
9552 {
9553   struct cmd *cmd;
9554 
9555   cmd = parseexec(ps, es);
9556   if(peek(ps, es, "|")){
9557     gettoken(ps, es, 0, 0);
9558     cmd = pipecmd(cmd, parsepipe(ps, es));
9559   }
9560   return cmd;
9561 }
9562 
9563 struct cmd*
9564 parseredirs(struct cmd *cmd, char **ps, char *es)
9565 {
9566   int tok;
9567   char *q, *eq;
9568 
9569   while(peek(ps, es, "<>")){
9570     tok = gettoken(ps, es, 0, 0);
9571     if(gettoken(ps, es, &q, &eq) != 'a')
9572       panic("missing file for redirection");
9573     switch(tok){
9574     case '<':
9575       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
9576       break;
9577     case '>':
9578       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9579       break;
9580     case '+':  // >>
9581       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9582       break;
9583     }
9584   }
9585   return cmd;
9586 }
9587 
9588 
9589 
9590 
9591 
9592 
9593 
9594 
9595 
9596 
9597 
9598 
9599 
9600 struct cmd*
9601 parseblock(char **ps, char *es)
9602 {
9603   struct cmd *cmd;
9604 
9605   if(!peek(ps, es, "("))
9606     panic("parseblock");
9607   gettoken(ps, es, 0, 0);
9608   cmd = parseline(ps, es);
9609   if(!peek(ps, es, ")"))
9610     panic("syntax - missing )");
9611   gettoken(ps, es, 0, 0);
9612   cmd = parseredirs(cmd, ps, es);
9613   return cmd;
9614 }
9615 
9616 struct cmd*
9617 parseexec(char **ps, char *es)
9618 {
9619   char *q, *eq;
9620   int tok, argc;
9621   struct execcmd *cmd;
9622   struct cmd *ret;
9623 
9624   if(peek(ps, es, "("))
9625     return parseblock(ps, es);
9626 
9627   ret = execcmd();
9628   cmd = (struct execcmd*)ret;
9629 
9630   argc = 0;
9631   ret = parseredirs(ret, ps, es);
9632   while(!peek(ps, es, "|)&;")){
9633     if((tok=gettoken(ps, es, &q, &eq)) == 0)
9634       break;
9635     if(tok != 'a')
9636       panic("syntax");
9637     cmd->argv[argc] = q;
9638     cmd->eargv[argc] = eq;
9639     argc++;
9640     if(argc >= MAXARGS)
9641       panic("too many args");
9642     ret = parseredirs(ret, ps, es);
9643   }
9644   cmd->argv[argc] = 0;
9645   cmd->eargv[argc] = 0;
9646   return ret;
9647 }
9648 
9649 
9650 // NUL-terminate all the counted strings.
9651 struct cmd*
9652 nulterminate(struct cmd *cmd)
9653 {
9654   int i;
9655   struct backcmd *bcmd;
9656   struct execcmd *ecmd;
9657   struct listcmd *lcmd;
9658   struct pipecmd *pcmd;
9659   struct redircmd *rcmd;
9660 
9661   if(cmd == 0)
9662     return 0;
9663 
9664   switch(cmd->type){
9665   case EXEC:
9666     ecmd = (struct execcmd*)cmd;
9667     for(i=0; ecmd->argv[i]; i++)
9668       *ecmd->eargv[i] = 0;
9669     break;
9670 
9671   case REDIR:
9672     rcmd = (struct redircmd*)cmd;
9673     nulterminate(rcmd->cmd);
9674     *rcmd->efile = 0;
9675     break;
9676 
9677   case PIPE:
9678     pcmd = (struct pipecmd*)cmd;
9679     nulterminate(pcmd->left);
9680     nulterminate(pcmd->right);
9681     break;
9682 
9683   case LIST:
9684     lcmd = (struct listcmd*)cmd;
9685     nulterminate(lcmd->left);
9686     nulterminate(lcmd->right);
9687     break;
9688 
9689   case BACK:
9690     bcmd = (struct backcmd*)cmd;
9691     nulterminate(bcmd->cmd);
9692     break;
9693   }
9694   return cmd;
9695 }
9696 
9697 
9698 
9699 
