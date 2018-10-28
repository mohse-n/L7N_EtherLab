**Q:** How to find out if the cycle frequency (FREQUENCY) is chosen too high?  
**A:** According to 1.5.2 documentation, warnings about "skipped" datagrams in the kernel log are a symptom.
___
In case of defining SYNC_REF_TO_MASTER, run the command below in terminal to observe 0.1 sec updates of the deviation of the reference slave's clock from the master's.
```bash
watch -n0 "ethercat reg read -p0 -tsm32 0x92c" 
```

