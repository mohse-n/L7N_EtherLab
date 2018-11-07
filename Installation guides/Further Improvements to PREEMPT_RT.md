## General Resources
I've found [the documentation for "Red Hat Enterprise Linux for Real Time"](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/) to be extremely well-organized and useful.   
The documentation consists of:  
* [Red Hat's guide to real-time programming](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/reference_guide/pref-preface).
* A guide to tuning aspects of a real-time linux system is [Red Hat Tuning Guide](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/). In particular, the entirety of [chapter 2](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/chap-general_system_tuning#Using_the_Tuna_interface) is definitely worth a look.

## Kernel Boot Parameters
Certain operating system configuration options are only tunable via the kernel command line.  
1. Prevent the CPU from going to sleep by
```
idle=poll processor.max_cstate=1
``` 
* Limiting the CPU to C1 at most doesn't allow for much idling and power saving.  
### References
* [Red Hat: Describes what RCU does in one sentence](https://access.redhat.com/solutions/2260151)   
* [Red Hat: Recommends above parameters](https://access.redhat.com/articles/65410)  
* [UT Blog: Briefly mentions processor.max_cstate](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning)  
2. Isolate a core (here core 1) for running only one task and offload housekeeping tasks from that CPU (system partitioning)  
```
isolcpus=1 nohz=on nohz_full=1 rcu_nocbs=1 rcu_nocb_poll intel_pstate=disable nosoftlockup 
``` 
Then assign a process to core 1 by setting its affinity in the code (sched_setaffinity).
* isolcpus removes the specified CPUs, as defined by the cpu_number values, from the general kernel SMP balancing and scheduler algroithms. 
The only way to move a process onto or off an "isolated" CPU is via the CPU affinity syscalls.
### References
* [Red Hat: Addional info on the parameters used for system partitioning](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/system_partitioning)
* [Red Hat: Offloading RCU Callbacks](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/offloading_rcu_callbacks)
* [Answer on Unix Stack Exchange: How to isolcpus](https://unix.stackexchange.com/questions/326579/how-to-ensure-exclusive-cpu-availability-for-a-running-process)  
* [Steven Rostedt's talk, nohz_full and rcu_nocbs, see 39:45](https://www.youtube.com/watch?v=wAX3jOHHhn0&t=2306s)  
* [UT blog: Explains rcu_nocbs](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning)  
* Search nosoftlockup in [this](https://access.redhat.com/sites/default/files/attachments/201501-perf-brief-low-latency-tuning-rhel7-v1.1.pdf) Red Hat document. 
3. The skew_tick=1 parameter causes the kernel to program each CPU's tick timer to fire at different times, avoiding any possible lock contention.
```
skew_tick=1 
```
* [Red Hat: Suggests skew_tick=1](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/reduce_cpu_performance_spikes)
* [Also on Red Hat's Bugzilla](https://bugzilla.redhat.com/show_bug.cgi?id=1451073)


