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
    kern_return_t writeto_multiple(const std::tuple<mach_vm_address_t, std::vector<uint8_t>> &pair, const std::optional<mach_port_t> &task={});

    /*
    Writes to an area of memory in a specified process in virtual memory
    - addr (mach_vm_address_t) -> Memory address to write to
    - bytes (std::vector<char>) -> Vector of bytes to write from
    - task (task_t) -> optional task
    */
    void writeto(const mach_vm_address_t addr, const std::vector<uint8_t> &bytes, const std::optional<task_t> &task={});

    // Don't use, thanks
    template <typename T> // template stuff fixed gg 
    void readfrom(const mach_vm_address_t addr, T* dest, const std::optional<task_t> &task={}) 
    {
        task_t t = (task.has_value()) ? task.value() : this->task;
        if(!t) { std::cerr << "No task to read from!" << std::endl; } // it'd be a good idea to stop the code exec here

        kern_return_t kerr;
        mach_msg_type_number_t placeholder = 0; // because no one apprarently has used mach_vm_read before

        // so placeholder is of course, a placeholder, I haven't found any resource explaining mach_vm_read but all I need is the last param
        if(kerr = mach_vm_read(t, addr, sizeof(T), reinterpret_cast<vm_offset_t*>(dest), &placeholder)); kerr != KERN_SUCCESS) {
            std::cerr << "Unable to read address " << std::hex << addr << std::endl;
            std::cerr << "Error: " << mach_error_string(kerr) << std::endl;
        }
    }

    // Verifies whether the user is running as superuser
    const bool hasPermissions();
};

#endif //MLIB_HPP
