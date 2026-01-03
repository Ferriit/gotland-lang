# Gotland language specification

## Keywords:
```
imp         IMPORT MODULE       Includes the [impl] and [data] blocks from a file. 'imp glb' imports a system library while 'imp loc' imports a library in the same directory.
glb         GLOBAL              Exposes a function to importing. Also used to import a system library.
loc         LOCAL               Only used for importing local libraries.

var         VARIABLE            Creates a variable. Global if in the [data] block and local if in a function.
fn          FUNCTION            Defines a function. Global if it's defined as 'glb fn'.
ret         RETURN              Returns a value from a function.

begin       {                   Begins a new scope.
end         }                   Ends the current scope.

if          IF                  Executes its scope if a thing is true.
NAME:       LABEL               Creates a label with the name 'NAME'. Note that 'NAME' can be replaced by anything and the case doesn't matter.
jmp         JUMP                Jumps to a label.

true        TRUE                A value of True, same as 0x01.
false       FALSE               A value of False, same as 0x00.

asm         ASSEMBLY            Inserts an assembly instruction.
```

## Blocks:
- There are three different blocks available, `[using]`, `[data]` and `[impl]`.
- The blocks are used to define different things.
- Only `[impl]` is necessary to create an executable. But `[data]` is required for strings, arrays and global variables and `[using]` is required to include other scripts.

### [using]
- This is where you put your imports with `imp loc` and `imp glb`.
- The compiler pastes everything this block requires in the other blocks. This means it copies the global implementations and data from the imported scripts.

### [data]
- This is where you define global variables, arrays and strings. Arrays and strings have to be defined here though.
- Variables here cannot be shadowed and become reserved.
- These variables can be accessed by scripts that import the current script and by every function in the script.

### [impl]
- This is where your function implementations go.
- The entrypoint is called `main`.
- Functions cannot go anywhere else, they have to be here.


## Variables:

### Types:
- Variables are almost always of type 'int', the only exceptions being strings, arrays and floating-point numbers. Strings and Arrays are prefixed with 'p' in their filenames to signal 'pointer' and floats are prefixed with 'f' to signify that.
- Arrays and Strings have to be defined in [data] to be valid. Thus making them global.
- All variables in [data] become global while everything else becomes local to a certain function. You cannot define variables inside of IF statements or outside functions and the [data] block.

- Booleans are False when 0 and True in every other case.

- Arrays and Strings can be defined like in C with `var parray[128]` without an initializing value.
- Strings are the same as arrays that store type char, which in turn is an integer.
- Chars are converted to ints based on ASCII.

- Floating point numbers that aren't a variable have to be suffixed with 'f' instead of prefixed.

* The list of all types is `array, int, char (int), bool (int), float`.

### Examples:
```
[data]
// These are global variables
var num1 = 10
var fnum2 = 10.3f     // Float
var c = 'a'         // Same as '61'

var parray[128]     // An array that's 128 indicies long
var pmsg = "Hello, world!"  // A string

var parray2 = [1, 2, 3, 4, 5, 6]

[impl]
fn main begin
    // These are local variables only accessable in this function. You cannot use variable names already defined in [data]
    var num3 = 5    // Cannot define arrays or strings inside a function
    var c2 = 'b'    // You can define characters though :D
end
```

## Functions:
- Functions aren't called with parentheses or commas. They're always called with 'functionname arg1 arg2 arg3'. You can therefore not do math with them directly, but have to assign them to a variable first.
- Functions are defined with 'fn' or 'glb fn' if it's a global function that you can use in another script. After that there's a function name and arguments separated by spaces using the same prefixes as everything else and lastly 'begin' to open a new scope.
- Functions always have to end with 'ret' and they need to end their scope with 'end'.
- Functions have to be defined in [impl] and can only be called from one another.
- The starting point is called 'main'.

```
[impl]
glb fn fadd fnum1 fnum2 begin
    // This is a global function that adds two floats together
    var fresult = fnum1 + fnum2
    ret fresult
end

fn main begin
    // This is a local function and the entrypoint of the program. Main cannot be global
    var fnum = fadd 3.14f 0.01f
    
    return 0
end
```

## Importing:
- You can import stuff either from your system, where the compiler looks in your PATH or from the same path as the script.
- To import system libraries, you do `imp glb packagename` where 'packagename' is the name of the library.
- To import local libraries, you do `imp loc packagename` where 'packagename' is the name of the library.
- All imports must be in the '[using]' block.

## Control flow:
- If statements check if what they have as their condition is true, and if so executed their scope.
- Ifs need a scope defined by `begin` and `end`.
- Jumps need a label to jump to.
- Labels are defined as `name:` where 'name' can be whatever you want. They're what jumps jump to.

## Semantics:
- Semicolons are only required when there are multiple statements on one line.
- Arguments to functions are whitespace separated and you cannot do math with functions, you have to assign their values to a variable first.


## Logging:
### Errors:

- Syntax Error template:
```
Syntax Error on line [LINE]:
Unknown token '[TRIGGERING_TOKEN]'
```

- Variable Out of Bounds template:
```
Out of Bounds Error on line [LINE]:
'[VARIABLE_NAME]' is not defined here
```

- Block Error template:
```
Block Error on line [LINE]:
'[TRIGGERING_TOKEN]' is in the wrong block
```

- Scope Error:
```
Scope Error on line [LINE]:
Scope is invalid
```

* If an error occurred, the compiler must also print `Compilation failed.` after all of the errors.
* When an error is encountered, compilation should continue until a full .asm file is generated. Then it can print all errors and stop compilation.

## Mics:
- Operand #1 should be stored in RAX and Operand #2 should be stored in RBX. Functions should take arguments on the stack in the order they appear but later switch to registers in the order RCX, RDX, R8 etc. etc. in the order they appear in the definition. Return values should be stored in RAX.
