## General Resources
I've found [the documentation for "Red Hat Enterprise Linux for Real Time"](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/) to be extremely well-organized and useful.
* [Red Hat's guide to real-time programming](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/reference_guide/pref-preface).
* A great guide to different aspects of tuning a real-time linux is [Red Hat Tuning Guide](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/). In particular, the entirety of [chapter 2](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/chap-general_system_tuning#Using_the_Tuna_interface) is definitely worth a look.

## Kernel Boot Parameters
1. Prevent the CPU from going to sleep by
```
idle=poll processor.max_cstate=1
``` 
* Limiting the CPU to C1 at most doesn't allow for much idling and power saving.  
### References
* [Red Hat: Describes what RCU does in one sentence](https://access.redhat.com/solutions/2260151)   
* [Red Hat: Recommends above parameters](https://access.redhat.com/articles/65410)  
* [UT Blog: Briefly mentions processor.max_cstate](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning)  
2. Isolate a core (here core 1) for running only one task (tickless)
```
isolcpus=1 nohz=on nohz_full=1 rcu_nocbs=1 intel_pstate=disable nosoftlockup
``` 
and then assign a process to core 1 by setting its affinity in the code (sched_setaffinity).
* isolcpus removes the specified CPUs, as defined by the cpu_number values, from the general kernel SMP balancing and scheduler algroithms. 
The only way to move a process onto or off an "isolated" CPU is via the CPU affinity syscalls.
### References
* [Red Hat: Addional info on the parameters](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/system_partitioning)
* [Answer on Unix Stack Exchange: How to isolcpus](https://unix.stackexchange.com/questions/326579/how-to-ensure-exclusive-cpu-availability-for-a-running-process)  
* [Steven Rostedt Talk, nohz_full and rcu_nocbs, see 39:45](https://www.youtube.com/watch?v=wAX3jOHHhn0&t=2306s)  
* [UT blog: Explains rcu_nocbs](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning)  



