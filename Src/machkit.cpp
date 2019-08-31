#include "machkit.hpp"

// work in progress code, major code & bug fixes will be done later 


/*
    Credit to: http://newosxbook.com/articles/PST2.html, major thanks!
    task_for_pid() workaround works for Mojave :3

    Code and certificate signing didn't work so cool
*/
mach_port_t task_for_pid_workaround(pid_t Pid)
{
    // Comments added here soon
    host_t        myhost = mach_host_self();
    mach_port_t   psDefault;
    mach_port_t   psDefault_control;

    task_array_t  tasks; 
    mach_msg_type_number_t numTasks;

    thread_array_t       threads;
    thread_info_data_t   tInfo;

    kern_return_t kr;

    kr = processor_set_default(myhost, &psDefault);
    kr = host_processor_set_priv(myhost, psDefault, &psDefault_control); 

    if (kr != KERN_SUCCESS) { fprintf(stderr, "host_processor_set_priv failed with error %x\n", kr); 
		mach_error("host_processor_set_priv",kr); exit(1);}

    kr = processor_set_tasks(psDefault_control, &tasks, &numTasks); 
    if (kr != KERN_SUCCESS) { fprintf(stderr,"processor_set_tasks failed with error %x\n",kr); exit(1); }

    for (int i = 0; i < numTasks; i++) {
        pid_t pid;
        pid_for_task(tasks[i], &pid);
        if (pid == Pid) return (tasks[i]);
    }

    return MACH_PORT_NULL;
}


/*
    default constructor, the arguments are optional, but one thing which the constructor does is 
    check permissions if the user can run the program ( which is needed for task_for_pid() and mach functions)
*/
machkit::machkit(const std::optional<std::string> &windowTitle) 
{
    this->task = MACH_PORT_NULL, this->base = 0, this->processId = 0;

    if(!this->hasPermissions()) {  // Permission stuff y'know the usual
        std::cout << "MachKit does not have enough permissions to run! Please run the binary as an superuser to continue!" << std::endl;
        std::exit(1);
    }

    if(windowTitle.has_value()) { // does bind if win title is passed in constructor so we won't waste time ig
        this->windowTitle = windowTitle.value();
        this->bind(windowTitle.value());
    }
}

// Daily owo

/*
    getuid() gets evidently the user's id, it compares to 0 because 0 is the uid of the root user
    which permissions are fundamental for the program to run
*/
const bool machkit::hasPermissions() 
{
    return (getuid() == 0) ? true : false;
}

// TODO: much more error checks
// Basically get process id, and task_t from window title
void machkit::bind(const std::string &winTitle) 
{
    if(!winTitle.empty()) {
        if(this->windowTitle.empty()) this->windowTitle = winTitle;

        pid_t procId = 0;
        std::array<pid_t, 2048> pids {};

        proc_listpids(PROC_ALL_PIDS, 0, &pids, sizeof(pids)); // proc_listpids returns a buffer of actual bytes for pids

        for(const auto &pid : pids) {
            proc_bsdinfo pinfo; // struct which contains info on the process
            proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &pinfo, sizeof(proc_bsdinfo)); // proc_pidinfo returns buffer of size of proc_bsdinfo(?)

            if(!strcmp(winTitle.c_str(), pinfo.pbi_name)) { // check if the name f the specified process is equal to the pid proc name
                procId = pid; // pinfo.pbi_pid works just as fine 
            }
        }
  
        if(procId > 0) { // nice nest
            this->processId = procId;
            this->task = task_for_pid_workaround(procId); // error checking is implementing in the function but I'll add more here later

            if(this->task > 0) {
                this->base = this->get_task_base(this->task);
            }
            else {
                std::cerr << "Error with retreiving the task!" << std::endl;
            }
        }
        else {
            std::cerr << "Error retrieving the proccess id!" << std::endl;
        }
    }
    else {
        std::cerr << "Error 'mlib::bind()':  There is no window to bind to!" << std::endl;
    }
}


// partially untested though compiles
void machkit::writeto(const mach_vm_address_t addr, const std::vector<uint8_t> &bytes, const std::optional<task_t> &task) // in case the user wants to write to a process manually 
{
    task_t t = (task.has_value()) ? task.value() : this->task; // Either we can specify our own task or the task generated from the constructor 
    if(!t) { std::cerr << "No task to write to!" << std::endl; } // check here haduadiahb kaiw
    
    kern_return_t kerr;

    // Changes protection so we can write to the memory address in virtual memory
    if(kerr = mach_vm_protect(t, addr, bytes.size(), FALSE, VM_PROT_READ | VM_PROT_WRITE); kerr == KERN_SUCCESS) { 
        if(kerr = mach_vm_write(t, addr, reinterpret_cast<vm_offset_t>(&bytes), bytes.size()); kerr != KERN_SUCCESS) {
            std::cerr << "Couldn't write process memory!" << std::endl;
            std::cerr << "MACH ERROR: " << mach_error_string(kerr) << std::endl;
        } 
    }
    else
    {
        std::cerr << "Was not able to complete protection change" << std::endl;
        std::cerr << "MACH ERROR: " << mach_error_string(kerr) << std::endl;
    }
}   

// untested
mach_vm_address_t machkit::get_task_base(const std::optional<task_t> &task)
{
    task_t t = task.has_value() ? task.value() : this->task;

    mach_vm_address_t vmoffset = 0; // base is here 
    vm_map_size_t vmsize;
    uint32_t nesting_depth = 0;
    vm_region_submap_info_64 vbr;
    mach_msg_type_number_t vbrcount = 16;
    kern_return_t kerr = mach_vm_region_recurse(t, &vmoffset, &vmsize, &nesting_depth, reinterpret_cast<vm_region_recurse_info_t>(&vbr), &vbrcount);

    if (kerr != KERN_SUCCESS)  { std::cerr << "Error: " << mach_error_string(kerr) << std::endl; }
    return vmoffset;
}

//experimental
// stuff
