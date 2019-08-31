#include "../Src/machkit.hpp"

int main()
{
    machkit mkit; // instantiate a new machkit object
    mkit.bind("GeometryDash.exe"); // we can either call the bind function manually or call it through the constructor

    mach_vm_address_t a = 0x2103428;
    int val;

    mkit.readfrom<int>(a, &val);
    std::cout << "Current attempt is " << val << std::endl;
}
