struct A {
    int a, b;
    A(void* v) {
        a = *(int*)v;
        b = a;
    }
};

int main() {
    int res = 0;
    if (res == 0) res = 1;
    else res=2;
    A a(&res);
    return a.b;
}