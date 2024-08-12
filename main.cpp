import helloc;

import <unordered_set>;
import <string>;

int main(int argc, char *argv[]) {
    std::unordered_multiset<std::string_view> ps;
    const int svArgc = argc;
    while (argc --> 1) {
        ps.insert(argv[svArgc-argc]);
    }
    return hello(ps);
}
