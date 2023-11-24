#include <algorithm>
#include "serpent.hpp"

const Serpent::Key key = { 0 };
const char* plaintext = "0123456789abcdef";

int main() {
    Serpent serpent(key);
    Serpent::Block block;
    std::copy_n(plaintext, sizeof block, block);

    serpent.encrypt(block);

    return 0;
}
