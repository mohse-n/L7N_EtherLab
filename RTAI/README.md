Basic structure of an EtherLab RTAI program:
1- Definition and initialization of variables
2- Definition of the cyclic task (run)
3- Kernel module stuff: __init and __exit and module info
__________________________________________________________________________

See RTAI documentation (availabe online and in the tarball) and IgH EtherCAT Master 1.1 Documentation for comments on the RTAI API.
__________________________________________________________________________
# RTAI Installtion Guide:
**Note:** This guide is derived from mainly [ShabbyX's](https://github.com/ShabbyX/RTAI/blob/master/README.INSTALL) and the official [guide](https://www.rtai.org/userfiles/downloads/RTAILAB/RTAI-TARGET-HOWTO.txt) on RTAI website (which is quite old).
## 1. Decide on a kernel version



## 2. Download the required files




## 3. Patch and build the kernel



## 4. Install RTAI in userspace





## 5. Run the latency test


