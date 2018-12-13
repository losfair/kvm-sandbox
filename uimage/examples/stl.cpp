#include <iostream>
#include <vector>

std::vector<int> v;

int main() {
    for(int i = 0; i < 1000; i++) v.push_back(i); // FIXME: implement brk & mmap
    std::cout << v.size() << std::endl;
    return 0;
}
