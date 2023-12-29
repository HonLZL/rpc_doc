#include <iostream>
#include <map>

int main() {
    std::multimap<int, std::string> myMultimap;

    myMultimap.insert(std::make_pair(1, "apple"));
    myMultimap.insert(std::make_pair(2, "banana"));
    myMultimap.insert(std::make_pair(1, "orange")); // 1键对应两个值

    // 输出multimap的内容
    for (const auto& pair : myMultimap) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    // 查找键为1的所有值
    auto range = myMultimap.equal_range(1);
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << "Key 1 has value: " << it->second << std::endl;
    }

    return 0;
}
