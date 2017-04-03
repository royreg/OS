3100 # Context switch
3101 #
3102 #   void swtch(struct context **old, struct context *new);
3103 #
3104 # Save current register context in old
3105 # and then load register context from new.
3106 
3107 .globl swtch
3108 swtch:
3109   movl 4(%esp), %eax
3110   movl 8(%esp), %edx
3111 
3112   # Save old callee-save registers
3113   pushl %ebp
3114   pushl %ebx
3115   pushl %esi
3116   pushl %edi
3117 
3118   # Switch stacks
3119   movl %esp, (%eax)
3120   movl %edx, %esp
3121 
3122   # Load new callee-save registers
3123   popl %edi
3124   popl %esi
3125   popl %ebx
3126   popl %ebp
3127   ret
3128 
3129 
3130 
3131 
3132 
3133 
3134 
3135 
3136 
3137 
3138 
3139 
3140 
3141 
3142 
3143 
3144 
3145 
3146 
3147 
3148 
3149 
