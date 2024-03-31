<!---
Essa análise foi desenvolvida ao som dessa bomba:
https://www.youtube.com/watch?v=qT6g7qlTjWE&t=6106s
-->
# OS contexts

## Variables
- Stacksize: 64\*1024 represents the size of the stack each context will use.

- ContextPing, ContextPong: both are u_context type variables.

## Functions
### getcontext
As described in ucontext.h manpage, getcontext initializes the structure pointed 
by the argument with the information of the current context.

### makecontext
This function addresses a function passed as argument to the u_context pointer.

### swapcontext
Saves current context on first argument and puts the second argument in 
execution

## Execution 
1. ContextPing is initialized with the current context.
2. stack is allocated, then it's passed as the pointed value in u_context
 component "uc_stack" in ContextPing. 
3. makecontext is called to address BodyPing function to it's corresponding 
u_context pointer.
4. The same 3 steps are applied to BodyPong, including a new allocation for 
BodyPong's stack.
5. swapcontext is called so ping proccess is put on execution and the current 
proccess is saved in ContextMain.
6. BodyPing alone should print "inicio", then iterate over a 4-step loop and 
print the number of each iteration and finally print "fim". Besides that, Ping
calls swapcontext in each iteration to put Pong in execution, and Pong prints 
the same thing. So the result of the execution is Ping printing one iteration, 
than Pong prints another till this repeats 4 times. Finally both end and call 
swapcontext to put main in execution again.

it's fun to notice that the program calls in main two "swapcontext", one for 
ContextPing and another for ContextPong, but because of the nature of the 
functions Ping and Pong they call each other and saves RIP in u_context 
structures, so you can maintain these 2 calls, as well as erase any one of them 
and the resulting output will be exactly the same.
