Basic structure of an EtherLab RTAI program:  
1- Definition and initialization of variables  
2- Definition of the cyclic task (run)  
3- Kernel module stuff: __init and __exit and module info  
___

See RTAI documentation (available online and in the tarball) and [IgH EtherCAT Master 1.1 Documentation](https://www.etherlab.org/download/ethercat/igh-ethercat-master-1.1.pdf) for comments on the RTAI API.
___
## RTAI Installtion Guide:
**Note:** This guide is derived from mainly [ShabbyX's](https://github.com/ShabbyX/RTAI/blob/master/README.INSTALL) and the official [guide](https://www.rtai.org/userfiles/downloads/RTAILAB/RTAI-TARGET-HOWTO.txt) on RTAI website (which is quite old).
### 1. Decide on a kernel version
There are two versions to take into account for detemining the kernel version:  
* **Igh EtherCAT Master:** The package has modified network card drivers only for specific versions of the kernel. 
* **RTAI**: Kernel patches are available for some kernels and not others. Also to consider is the fact that newer releases (e.g. RTAI 5.1) are not widely adopted and therefore not thoroughly tested and debugged.   
#### Igh EtherCAT Master
Since we're going to write IgH EtherCAT Master (from here on called IgH EtherCAT) programs, we start from there.  
The current IgH EtherCAT version is 1.5.2 and can be downloaded from the [website](https://www.etherlab.org/en/ethercat/index.php).  
**Note:** The [SourceForge repository](https://sourceforge.net/projects/etherlabmaster) is regularly updated and is much more recent, but during this walkthrough, we stick to the tried-and-tested versions due to better stability and support from the community.  
Looking at the "devices" folder, we can see the modified (and original) drivers and their associated kernel version.
For instance,    
r8169-3.4-ethercat.c
is the modified driver for Realtek8169 family of network cards for kernel 3.4.x .
**Note:** Drivers for the more recent kernels exist on the [SourceForge repository](https://sourceforge.net/projects/etherlabmaster) and Gavin Lambert's [unofficial patchset](https://sourceforge.net/u/uecasm/etherlab-patches/ci/default/tree/#readme).  




### 2. Download the required files




### 3. Patch and build the kernel



### 4. Install RTAI in userspace





### 5. Run the latency test


