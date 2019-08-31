#ifndef MACHKIT_HPP
#define MACHKIT_HPP

// Includes
#include <iostream>
#include <vector>
#include <optional>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <dlfcn.h>
#include <unistd.h>
#include <libproc.h>
#include <array>

/*
Working alternative to task_for_pid
- pid (pid_t) -> process id of a running process
*/
mach_port_t task_for_pid_workaround(pid_t Pid);

// Mach-O Library Class 
class machkit
{
private:

public:
    // Regular info
    std::string windowTitle;
    pid_t processId;

    // Mach specific 
    mach_port_t task;
    mach_vm_address_t base;

    /*
    Default mlib class constructor 
    - WindowTitle (std::string) -> Name of the process window
    */
    machkit(const std::optional<std::string> &windowTitle={});

    // I plan to maybe change these voids to bools so I can do more proficient error checking

    /*
    Retrieves program information of a specific window title
    - winTitle (std::string) -> Name of the process window
    */
    void bind(const std::string &winTitle);

    /*
    Get base address in virtual memory of a certain process given by its task
    - task (task_t) -> task of a running process
    */
    mach_vm_address_t get_task_base(const std::optional<task_t> &task={});

    // undefined temporarly
    kern_return_t writeto_multiple(const std::tuple<mach_vm_address_t, std::vector<unsigned char>> &pair, const std::optional<mach_port_t> &task={});

    /*
    Writes to an area of memory in a specified process in virtual memory
    - addr (mach_vm_address_t) -> Memory address to write to
    - bytes (std::vector<char>) -> Vector of bytes to write from
    - task (task_t) -> optional task
    */
    void writeto(const mach_vm_address_t addr, const std::vector<unsigned char> &bytes, const std::optional<task_t> &task={});

    template <class T>
    void readfrom(const mach_vm_address_t addr, T* dest, const std::optional<task_t> &task={});

    // Verifies whether the user is running as superuser
    const bool hasPermissions();
};

#endif //MLIB_HPP
