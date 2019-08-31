#include "../Src/machkit.hpp"

int main()
{
    machkit mkit; // instantiate a new machkit object
    mkit.bind("firefox"); // we can either call the bind function manually or call it through the constructor

    // Example
    std::cout << mkit.windowTitle << " has pid " << mkit.processId << std::endl;
    std::cout << "Base is: 0x" << std::hex << mkit.base << std::endl;
}