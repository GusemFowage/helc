//int sum(int a, int b, int c) {
//    return a*b+c;
//}
//int walk(int n) {
//    if(n==1)
//        return 1;
//    else if(n==2)
//        return 2;
//    else
//        return walk(n-1)+walk(n-2);
//    // walk(5) -> walk(4) + walk(3) = 8
//    /*
//     * walk(4) = walk(3) + 2 = 5
//     * walk(3) = 2 + 1 = 3
//     */
//}
//int main() {
//    // 1 * 3 + 2 = 5;
//    int a = sum(1, 3, 2);
//    a = walk(({
//        int i = a, j = 1;       // i = 5, j = 1
//        i / j;                  // expr := 5 / 1 = 5
//    }));
//    return a;
//}

int main() {
    int a = 10;
    int * p = &a;
    *p = 8;
    return a;
}