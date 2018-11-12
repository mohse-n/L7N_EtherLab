### General Resources
I've found [the documentation for "Red Hat Enterprise Linux for Real Time"](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/) to be extremely well-organized and useful.   
The documentation consists of:  
* [Red Hat's guide to real-time programming](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/reference_guide/pref-preface).
* A guide to tuning aspects of a real-time linux system is [Red Hat Tuning Guide](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/). In particular, the entirety of [chapter 2](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/chap-general_system_tuning#Using_the_Tuna_interface) is definitely worth a look.
* [Improving the Real-Time Properties](http://linuxrealtime.org/index.php/Improving_the_Real-Time_Properties) is a collection of best practices that covers much of the same ground as the tuning guide by Red Hat (and perhaps a bit more), but it does recommend different values for a few parameters.
### Ubuntu Installation
Install Ubuntu with `ext2` file system.   

**References**    
* [Red Hat: File System Determinism Tips](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/file_system_determinism_tips)  
### BIOS Settings
Disable any power saving features.   

**References**     
* [Red Hat: Setting BIOS parameters](https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_MRG/1.3/html/Realtime_Tuning_Guide/sect-Realtime_Tuning_Guide-General_System_Tuning-Setting_BIOS_parameters.html)
### Kernel Boot Parameters
Certain operating system configuration options are only tunable via the kernel command line.  
#### 1. Disable CPU power saving mode
```
idle=poll processor.max_cstate=1
``` 
* Limiting the CPU to `C1` power mode doesn't allow for much idling and power saving.    

**References**   
* [Red Hat: Describes what RCU does in one sentence](https://access.redhat.com/solutions/2260151)   
* [Red Hat: Recommends above parameters](https://access.redhat.com/articles/65410)  
* [UT Blog: Briefly mentions processor.max_cstate](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning) 
#### 2. Parameters for CPU Isolation
Isolate a core (here core 1) for running only one task and offload housekeeping tasks from that CPU. 
```
isolcpus=1 nohz=on nohz_full=1 rcu_nocbs=1 rcu_nocb_poll intel_pstate=disable nosoftlockup 
``` 
We will later assign a process to core 1 by setting its affinity in the code (via `sched_setaffinity`).
* `isolcpus` removes the specified CPUs, from the general kernel SMP balancing and scheduler algroithms. 
The only way to move a process onto or off an "isolated" CPU is via the CPU affinity syscalls.
___
**Note:** To list the paramerts passed by the bootloader to the kernel,
```bash
cat /proc/cmdline
``` 
___
**Note:** The `PID` column in the output of these commands is the process' ID. The first command displays that for userspace processes only, whereas the second command also lists kernel space processes.
```bash
ps -aF
``` 
```bash
ps -eF
``` 
To get the CPU affinity mask of a process with ID PID,
```bash
taskset -p PID
``` 
Note that the output of the command above is the CPU affnity mask of the processs, which is described as a hexadecimal number, and should be interpreted as a binary number. For instance if the output is (1=) 0x1 = 01b, the process is bound to CPU 0.
___
**Note:** We can check whether the isolated CPU is running in tickless mode by looking at the number of `Local timer interrupts` that it handles. Either that figure should not increase at all (ideal) or it should increase by 1 per second ([turns out achieving the ideal rate of zero is not trivial](https://lwn.net/Articles/659490/)) . To list the total number of interrupts handled by each CPU,
```bash
cat /proc/interrupts
``` 
In my experience, CPU 1 -which doesn't start with any processes with our configuration- started with 1 tick per second, but unfortunately, once assigned a process (with `sched_setaffinity` in the user example), the scheduler ticks increased to some number in 250-300 range (the tick rate is adaptive, after all). 
This behaviour is something worth investigating.    
One possiblity: maybe it's due to 3.4.113 not having a `CONFIG_NO_HZ_FULL` option, in contrast to newer kernels? Note that in the [kernel documentation](https://www.kernel.org/doc/Documentation/timers/NO_HZ.txt),   

* for `CONFIG_NO_HZ=y`, which is the only option on 3.4: "Omit scheduling-clock ticks on idle CPUs"
* for `CONFIG_NO_HZ_FULL=y`, which is not available on 3.4: "Omit scheduling-clock ticks on CPUs that are either idle **or that have only one runnable task.**"  

**References**   
* [nohz_full=godmode ?](https://jeremyeder.com/2013/11/15/nohz_fullgodmode/) discusses various parameters that affect tickless operation.
* [Red Hat: Addional info on the parameters used for system partitioning](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/system_partitioning)
* [Red Hat: Offloading RCU Callbacks](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/offloading_rcu_callbacks)
* [Kernel NO_HZ documentation](https://www.kernel.org/doc/Documentation/timers/NO_HZ.txt). The interesting part (for us) starts from "OMIT SCHEDULING-CLOCK TICKS FOR IDLE CPUs".
* [Answer on Unix Stack Exchange: How to isolcpus](https://unix.stackexchange.com/questions/326579/how-to-ensure-exclusive-cpu-availability-for-a-running-process)  
* [Steven Rostedt's talk, nohz_full and rcu_nocbs, see 39:45](https://www.youtube.com/watch?v=wAX3jOHHhn0&t=2306s)  
* [UT blog: Explains rcu_nocbs](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning)  
* Search nosoftlockup in [this](https://access.redhat.com/sites/default/files/attachments/201501-perf-brief-low-latency-tuning-rhel7-v1.1.pdf) Red Hat document.   
The `skew_tick=1` parameter causes the kernel to program each CPU's tick timer to fire at different times, avoiding any possible lock contention.
```
skew_tick=1 
```
**References**   
* [Red Hat: Suggests skew_tick=1](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/reduce_cpu_performance_spikes)
* [Also on Red Hat's Bugzilla](https://bugzilla.redhat.com/show_bug.cgi?id=1451073)
### Prevent IRQ Handling by CPU 1
Continuing along the CPU isolation patch, we can delegate interrupt handling to our CPU of choice.
Disable `irqbalance` daemon, which distributes IRQ handling among CPUs.
```bash
nano /etc/default/irqbalance
```
in this file, to disable the daemon, add (or set)
```
ENABLED="0"
```
And to exclude CPU 1 from handling IRQs,
```
IRQ_BALANCE_BANNED_CPUS="2"
```
The second reference explains for why we set the value to "2".
___
**Note:** Interrupts handled by each CPU are listed in `/proc/interrupts`
```bash
cat /proc/interrupts
``` 
It's also possilbe to monitor `/proc/interrupts` using `watch`
```bash
watch -n 1 "cat /proc/interrupts"
```
If all the configurations up to this point are applied, the number of interrupts handled by CPU 1 should be 0 -or at least constant- for all entries. However, in my setup there has been three exceptions, namely `Local timer interrupt` (discussed above), `Machine check polls` and `Function call interrupts`. The latter case is strange, as the number of instances increased for CPU 1 and stayed constant at 0 for CPU 0. Disabling `Machine check polls` is describd later in this guide.  
`Function call interrupt` is seemingly an architecture-specific interrupt, and there might not be much we can do about it.
    
___
**References**    
* [Red Hat: Interrupt and Process Binding](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/interrupt_and_process_binding)
* [IRQBALANCE_BANNED_CPUS explained](https://fordodone.com/2015/04/30/irqbalance_banned_cpus-explained-and-working-on-ubuntu-14-04/)
* [Function call interrupts are an example of inter-processor interrupts.](https://wiki.linaro.org/WorkingGroups/PowerManagement/Doc/WakeUpSources#IPI4_:_Single_function_call_interrupts)
### Set the default CPU for handling IRQs
*I don't think this and the previous configuration are not necessarily mutually exclusive.*
Unwanted interrupts introduce jitter and can have serious negative impact on real-time performance. They should be handled on the general purpose CPUs whenever possible. The affinity of these interrupts can be controlled using the `/proc` file system.  
Write CPU 0's mask to `/proc/irq/default_smp_affinity`.
```bash
echo 1 > /proc/irq/default_smp_affinity
``` 
**References**
* [Improving the Real-Time Properties](http://linuxrealtime.org/index.php/Improving_the_Real-Time_Properties#Restart_real-time_CPUs_with_CPU_hotplug) suggests setting `default_smp_affinity`.
* [Red Hat: Interrupts and IRQ Tuning](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/performance_tuning_guide/s-cpu-irq) and [Red Hat: Interrupts and Process Binding](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/interrupt_and_process_binding) demonstrate the steps so that a particular set of CPUs handle specific interrupts.
### Set Various Kernel Parameters in /etc/sysctl.conf  
The sysctl command is used to modify kernel parameters at runtime. `/etc/sysctl.conf` is a text file containing sysctl values to be read in and set by sysct at boot time. ([Source](https://www.cyberciti.biz/faq/linux-kernel-etcsysctl-conf-security-hardening/))
We set the value of the following parameters, as recommended by Red Hat.
* `kernel.hung_task_timeout_secs = 600`: Sets timeout after a task is considered hanging to 600 seconds.
* `kernel.nmi_watchdog = 0`: Disables NMI's (non-maskabled interrupts) watchdog, which is used to detect and recover from hardware hangings.
* `kernel.sched_rt_runtime_us = 1000000`: Disable real-time throttling. In other words, dedicate 100% of CPU time to the real-time tasks, until the finish or yield. This is OK as long as the RT task are running on isolated cores.
* `vm.stat_interval = 10`: Increase the time interval between which vm (virtual memory) statistics are updated to 10 seconds. The default
is 1 second.
```bash
nano /etc/sysctl.conf
``` 
Add somewhere,
```
kernel.hung_task_timeout_secs = 600
kernel.nmi_watchdog = 0
kernel.sched_rt_runtime_us = 1000000
vm.stat_interval = 10
``` 
An additional configurations is disabling watchdog timer, which generates a hardware interrupts for detecting software lockups in the kernel. For more info, see the help of LOCKUP_DETECTOR parameter.  
```
kernel.watchdog = 0
``` 
**References**   
* [Red Hat: System Partitioning](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/system_partitioning) is where these settings are recommended.
* [Explains kernel timeout](https://www.nico.schottelius.org/blog/reboot-linux-if-task-blocked-for-more-than-n-seconds/)
* [What does an NMI watchdog do?](https://unix.stackexchange.com/questions/353895/should-i-disable-nmi-watchdog-permanently-or-not)
* [Real-time Linux from a Basic Perspective: Real-time Throttling](http://linuxrealtime.org/index.php/Basic_Linux_from_a_Real-Time_Perspective)
* [Kernel documentation: vm](https://www.kernel.org/doc/Documentation/sysctl/vm.txt)
* [Improving the Real-Time Properties: disable the watchdog and NMI watchdog](http://linuxrealtime.org/index.php/Improving_the_Real-Time_Properties#Disable_the_watchdog)
### Affine the bdi-flush Threads to CPU 0 
This configuration has be appiled at startup.
```bash
echo 1 > /sys/bus/workqueue/devices/writeback/cpumask
``` 
**References** 
* [nohz_full=godmode?](https://jeremyeder.com/2013/11/15/nohz_fullgodmode/)
* [Red Hat: System Partitioning](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/system_partitioning)
### Disable Machine Check Polls
This configuration has be appiled at startup. For CPU 1,  
```bash
echo 1 >  /sys/devices/system/machinecheck/machinecheck1/ignore_ce 
``` 
**References** 
* [nohz_full=godmode?](https://jeremyeder.com/2013/11/15/nohz_fullgodmode/)
* [Red Hat: System Partitioning](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/system_partitioning)
### Load Dynamic Libraries at Application Startup
Although it can slow down program initialization, it is one way to avoid non-deterministic latencies during program execution.
```bash
LD_BIND_NOW=1
``` 
```bash
export LD_BIND_NOW
``` 
**References**    
* [Red Hat: Loading Dynamic Libraries](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/loading_dynamic_libraries)








