This repository contains example code (orginiated from and similar to the examples in IgH EtherCAT Master library), 
written for motion control of two servomotors connected to LS Mecapion L7N servo drives in CSP mode. The codes are extensively 
commented to facilitate the conversion process to other setups.  
Additionally, in "Installation guide" folder, you will find a step-by-step and tested procedure for installing RTAI and subsequently
IgH Master.    
___
To run the programs, after installing the IgH Master library, copy-and-replace each in the "examples" folder, and 
1. For the user example, in "/usr/local/src/ethercat/examples/usr",
   - "make clean" 
   - "make"   
   - "./ec_user_example" to run the userspace process. 
2. For the RTAI example, in "/usr/local/src/ethercat/examples/rtai",
   - "make clean" 
   - "make modules"  
   - "insmod ec_rtai_sample.ko" to load the kernel module (and start the hard real-time task).


