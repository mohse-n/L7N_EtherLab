## Kernel Boot Parameters
1. Prevent the CPU from going to sleep by
```
idle=poll processor.max_cstate=1
``` 
* Limiting the CPU to C1 at most doesn't allow for much idling and power saving.  
[Recommended by redhat](https://access.redhat.com/articles/65410)  
[Briefly mentions processor.max_cstate](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning)  
2. Isolate a core (here core 1) for running only one task (tickless)
```
isolcpus=1 nohz_full=1 rcu_nocbs=1
``` 
and then assign a process to core 1 by setting its affinity in the code (sched_setaffinity).
* isolcpus removes the specified CPUs, as defined by the cpu_number values, from the general kernel SMP balancing and scheduler algroithms. 
The only way to move a process onto or off an "isolated" CPU is via the CPU affinity syscalls.  
[How to isolcpus](https://unix.stackexchange.com/questions/326579/how-to-ensure-exclusive-cpu-availability-for-a-running-process)  
[nohz_full and rcu_nocbs, see 39:45](https://www.youtube.com/watch?v=wAX3jOHHhn0&t=2306s)  
[Explains rcu_nocbs](https://utcc.utoronto.ca/~cks/space/blog/linux/KernelRcuNocbsMeaning)  



