# Hello C
helC is a C language compiler 
## Why to choose helC?
The purpose is to help green hands understand the process of C compilation; 
## How to use helC?
helC isn't a completed C compiler, so you can't to compile a real C source;
but, you can try this way;
``` C
/// check.c
#include <stdio.h>
extern int _main();
int main() {
    printf("%d", _main());
    return 0;
}
```
``` C
/// hello.c
int walk(int n) {
    if (n<=1) return 1;
    else
        return walk(n-1) + walk(n-2);
    /**
     * walk(5) = walk(4) + walk(3)
     * ...
     * => 8
     */
}
int main() {
    int a = 5;
    int * p = &a;
    // a = 5
    // then p = 8
    *p = walk(a);
    return a;
    // will return 8
}
```
``` shell
# run.sh
# choose the way you know to compile this project
mkdir build
cd build
cmake ..
# find the executable to run
cd app/Debug
hello.exe test.c -o test.s
# choose the compiler you use
# link the check.c and the source like this
gcc check.c test.s -o a.exe
# run the executable file
./a.exe
```
the way to [Use](docs/HowToUse.md "See the way to use") in detail 
