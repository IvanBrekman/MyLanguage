# SimpleProcessor
Project implements work of simple processor.

## Structure
Project has 4 independent programs:

1. **Assembler**: read commands from source file, checks them, and if they are correct - convert them to binary codes and write to executable file.
2. **Disassembler**: doing reverse work: read binary commands from executable file and convert them to text asm commands, then write text commands to decompile_output file (see appropriate variable at run_cpu.cpp).
    * `!Note!` In source_file with commands could be used labels with unique names, but information about their names dont saves in binary codes, so disassembler cant recover original names, so program will generate labels like "label_n", where n is number of label.
3. **Processor**: main program, which read binary commands from executable file and execute all commands.
4. **Project builder**: program compile 3 other programs (if it need), then compile\decompile source_file using assembler\disassembler (if it need) and finally execute file using processor.

## Write programs
**Source file** - file with asm commands, which defined in *arch/commands_deffinition.h*.

There are different blocks of commands
1. **Break commands**: contains commands for exit from program
    * hlt - normal exit from program. `!Note!` All programs must ends with *hlt* command. If processor wont execute this command behavior is undefined, but program will try to execute all commands and finished with exit code *"No obvious end"*.
    * abrt - intentional abort from program. Program will finished with bad exit code.

2. **Stack commands**: contains commands for work with processor stack
    * push - push value to stack. There are different arg types for this command:
        + *push const*            (push const value)
        + *push register*         (push value from register)
        + *push register+const*   (push sum of value from register and const value)
        + *push [const]*          (push value from const value cell of RAM)
        + *push [register]*       (push value from register value cell of RAM)
        + *push [register+const*] (push value from sum of register and const value cell of RAM)
    * pop - pop value from stack. Allowed arguments types:
        + *pop*                   (pop value from stack)
        + *pop register*          (pop value from stack and save it to register)
        + *pop [const]*           (pop value from stack and save it to const value cell of RAM)
        + *pop [register]*        (pop value from stack and save it to register value cell of RAM)
        + *pop [register + const]*(pop value from stack and save it to sum of register and const value cell of RAM)
    * vrf - verify processor stack. Command called Stack_error function and print it result.
    * dump - call dump of processor stack.
    * prt - print processor stack (not as informatice as dump).
3. **Arithmetic commands**: contains commands, which take values from processor stack, execute an arithmetic operation and push result back to stack.
    * add - adds 2 number from stack.
    * sub - substract 2 values from stack. `!Note!` Reduced number will be a number, which will pop second from stack.
    * mul - multiplicate 2 values from stack
    * div - devide 2 values from stack. `!Note!` Numerator will be a second poped value. If first popped value will be 0 program wil be finished with intentional abort.
    * mod - take mode from stack value on mod argument.
        + *mod const*           (stack.pop % const value)
    * sqr - take value from stack and push back value in square.
    * sqrt - take value from stack, calucate sqrt from it and push it back like int number. `!Note!` If number will be < 0 program will be finished with intentional abort.
    * abs - take value from stack and calucate absolute from this number and push it back.
4. **IO commands**: contains commands for IO.
    * in - keyboard input number and push it to stack. `!Note!` If user enter not a number, command will clear stdin and waiting new input from keyboard.
    * inv - similar with *in* command, but push in stack input value * 10^precision. `!Note!` Precision is a value in *$prec* register, default value of *$prec* register is 0.
    * out - pop value from stack and outputs it to stdout.
    * outv - similar with *out* command, but output value * 10^precision.
5. **Jump-types commands**: contains commands of jumps and call.
    * jmp - standart jump function, which set procesor ip to new value. This value is set by label.
    * jg - takes 2 values from stack and execute jmp if second value > first value.
    * jl  - same behavior. Jmp if second value  < first value.
    * je  - same behavior. Jmp if second value == first value.
    * jne - same behavior. Jmp if second value != first value.
    * jge - same behavior. Jmp if second value >= first value.
    * jle - same behavior. Jmp if second value <= first value.
    * call - save return address to processor call stack and execute jmp.

    *all jmp commands and call command needs 1 argument - label, where need to jump* : jmp customLabel_name
    * ret - pop value from processor call stack, and execute jmp to pop value.
6. **Draw commands**: contains draw commands at console.
    * draw - command takes data from VRAM and draw symbols in console. Screen size sets by 2 args: height and weight.
        + draw const const
        + draw register register
        + draw register+const register+const
        + draw [const] [const]
        + draw [register] [register]
        + draw [register+const] [register+const]
        + And also any combinations of possible types of args. For example draw const [register+const]
    * cat - command draw puzzled cat in a console :)

You can write different programs using this command system. Program examples you can find at *programs* dir. But tou can also add new command in system, if you need it

## Add commands
To add command go to *arch/commands_definition.h* file and write new macros using DSL or directly accessing to processor and other commands.

You can find instructions for adding command in *arch/commands_definition.h*.

Instructions for adding new DSL word are located in *arch/DSL/caommands_syntax.h*.

## Preferences
Global constants defining processor work located at *config.h*.

Such as:
1. *Header*: structure of binary header in executable file
2. *Max filepath size*: max length of path to all files
3. *Max arg size*: max length of string, which containes 1 argument of command
4. Others...

Full description find in specified file.

Different constants influencing on processor limits located at *arch/helper.h* file.

Such as:
1. *Max labels*: amount of maximum labels amount
2. *RAM size*: size of processor RAM
3. *VRAM start*: start index of VRAM in processor RAM
4. Others...

Full description find in specified file.

## Run
    1. First way is manual calling all 3 programs
    To run your program you need to call *assembler program* (compile asm/asm.cpp before if it need) and create executable file.

    Then call *disassembler program* (compile dis/dis.cpp before if it need) if you want to get your disassembled program and check correctness of assembling.

    Last step is to call *cpu program* (compile CPU/cpu.cpp before if it need) and execute your executable_file
.

    2. Second way is to call *main.cpp* with will call run_cpu function (compile it before if it need).
    This function checks tracked programs and compile assembler, disassembler and processor programs, if it need**.
    After that it checks your program source_file and recompile it, if it need**. Finally, it call processor program and execute file.

    ** Function checks time of last changing tracked programs.

## Last words
I hope you will enjoy using this processor. Write your comments and suggestions about this program and create issuies if you find bugs :)

Author of this project - Ivan Brekman
