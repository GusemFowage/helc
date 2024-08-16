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