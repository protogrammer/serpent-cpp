#include <iostream>
#include <algorithm>
#include "serpent.hpp"

const Serpent::Key key = { 0 };
const char* plaintext = "0123456789abcdef";
const unsigned char zeros[16] = { 0 };

int main() {
    std::cout << std::hex;

    std::cout << "Key:";
    for (auto x: key)
        std::cout << ' ' << (int)x;
    std::cout << std::endl;

    Serpent serpent(key);
    Serpent::Block block;
    std::copy_n(zeros, 16, block);

    std::cout << "Block before encryption:";
    for (auto x: block)
        std::cout << ' ' << (int)x;
    std::cout << std::endl;

    serpent.encrypt(block);

    std::cout << "Block after encryption: ";
    for (auto x: block)
        std::cout << ' ' << (int)x;
    std::cout << std::endl;

    serpent.decrypt(block);

    std::cout << "Block after decryption: ";
    for (auto x: block)
        std::cout << ' ' << (int)x;
    std::cout << std::endl;

    return 0;
}
