# The way to use
> hello.exe 'Source' -o 'OutputFile'
> > - 'Source': default is 'test.c' <br>
> > - 'OutputFile': default is 'stdout'
## The functions have been supported
- [x] define a **function** | **variable**
- [x] use **type**(only "int")
- [x] support **pointer**("int*")
- [x] some **math** function like (a+b, a-b, a*b, a/b)
- [x] some **compare** function like (a<b, a>b, a<=b, a>=b, a == b)
- [x] **while**, **do-while**
- [ ] **for** function isn't completed enough
- [ ] ...
## Example Source
``` C
/// test.c
int main() {
    int sum = 0, i = 1;
    while (i <= 10) {
        sum = sum + i;
    }
    return sum;
}
/// output: 55
```
``` C
/// test.c
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
// output: 8
```