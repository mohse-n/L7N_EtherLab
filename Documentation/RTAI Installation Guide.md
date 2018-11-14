
## RTAI Installtion Guide

**Note:** The following guide has been tested on Ubuntu 14.04 LTS (gcc version 4.8.4).
___
**Note:** This guide is derived mainly from Juan Serna's excellent [tutorial](https://sites.google.com/site/thefreakengineer/tutorials/rtai-5-0-1-lubuntu-14-04-x64) 
___
### 1. Decide on a kernel version
There are two versions to take into account when detemining the kernel version:  
* **Igh EtherCAT Master:** The package has modified network card drivers only for specific versions of the kernel. 
* **RTAI**: Kernel patches are available for some kernels and not others. Also to consider is the fact that newer releases (e.g. RTAI 5.1) are not widely adopted and therefore not thoroughly tested and debugged.   
#### Igh EtherCAT Master
Since we're going to write IgH EtherCAT Master (from here on called IgH Master) programs, we consider its version first and proceed accordingly. 
The lastest stable version of the library can be downloaded from the [SourceForge repository.](https://sourceforge.net/p/etherlabmaster/code/ci/stable-1.5/tree/). 
Looking at the `devices` folder, we can see the modified (and original) drivers and their associated kernel versions.
For instance,  
```
r8169-3.4-ethercat.c  
``` 
is the modified driver for Realtek8169 family of network cards for kernel 3.4.x . Keep 3.4 in mind and move on to the next section (RTAI).  
___
**Note:** Drivers for the more recent kernels are available in Gavin Lambert's [unofficial patchset](https://sourceforge.net/u/uecasm/etherlab-patches/ci/default/tree/#readme).  
___
#### RTAI
Now we should look for a version of RTAI that has a patch for kernel 3.4.x . The existence of .patch file should be checked by downloading the package and looking in the directory  
```bash
/base/arch/x86/patches 
``` 
In my case, I downloaded RTAI 4.0 and there were two .patch files for 3.4.x kernels:  
```bash
hal-linux-3.4.6-x86-4.patch  
```
```bash
hal-linux-3.4.67-x86-4.patch 
```     
Thus, we basically have to decide between kernel 3.4.6 and 3.4.67. I chose the base version (3.4.6) (for no particular reason).  
### 2. Download the required files
```bash
sudo -s
```
We use cURL for downloading RTAI and Linux kernel.
```bash
apt-get install curl
```
Kernels have to exist at `/usr/src`, and we're going to download everything to that directory.
```bash
cd /usr/src
```
Download the appropriate version of RTAI,
```bash
curl -L https://www.rtai.org/userfiles/downloads/RTAI/rtai-4.0.tar.bz2 | tar xj
```
Download the associated kernel,
```bash
curl -L https://www.kernel.org/pub/linux/kernel/v3.x/linux-3.4.6.tar.xz | tar xJ
```
When building the RTAI-patched kernel, we're going start from the default kernel configuration. That way we'll be sure that the only cause of possbile failure in kernel boot process is the modification that **we** have made.
___
**Note:** Entering the correct URL below requires a visit to http://kernel.ubuntu.com/~kernel-ppa/mainline.
___
```bash
curl -L http://kernel.ubuntu.com/~kernel-ppa/mainline/v3.4.6-quantal/linux-image-3.4.6-030406-generic_3.4.6-030406.201207191609_amd64.deb -o linux-image-3.4.6-generic-amd64.deb
```
Extract the .deb package. 
```bash
dpkg-deb -x linux-image-3.4.6-generic-amd64.deb linux-image-3.4.6-generic-amd64
```
Install the packges needed for building the kernel.
```bash
apt-get update
```
```bash
apt-get install libncurses5-dev dpkg-dev
```

### 3. Patch, configure, and build the kernel
Replace the default Ubuntu `.config` file with the configuration file of the associated Ubuntu kernel,
```bash
cp /usr/src/linux-image-3.4.6-generic-amd64/boot/config-3.4.6-030406-generic /usr/src/linux-3.4.6/.config
```
Apply the RTAI patch to the kernel source files,
```bash
cd /usr/src/linux-3.4.6
```
```bash
patch -p1 < /usr/src/rtai-4.0/base/arch/x86/patches/hal-linux-3.4.6-x86-4.patch
```
Now we're ready to configure the kernel.
```bash
make menuconfig
```
1. Disable `Enable loadable module support > Module versioning support ` (IgH Master RTAI modules wouldn't compile otherwise)
2. If you're using a 64-bit CPU: `Processor type and features > Processor family > Generic x86_64`
3. Number of physical cores (i.e. not accounting for hyperthreading): `Processor type and features > Maximum number of CPU’s > 2` (My PC had i3-4700, which has 2 physical cores)
4. Disable `Processor type and features > SMT (Hyperthreading) scheduler support`
5. Disable `Processor type and features > Tickless System (Dynamic Ticks)`
6. Choose `Processor type and features > Timer frequency (1000 HZ)`
7. Under `Power management and ACPI options`, disable anything that you can, including `CPU Frequency Scaling` and `CPU idle PM support`.
8. Under `Power management and ACPI options > ACPI`, disable everything you're able to, except `Power Management Timer Support` and `Button`.  
9. Select `Exit` and save.  
___
**Note:** Also worth checking are the various guides and recommendations for the optimal kernel configuration in linuxcnc website and forum.
___
Now we should be able to compile the kernel. Note that since many, many device drivers are enabled and will be compiled, and, additionally, since dpkg doesn't support multithreading, the installation process takes a significant amount of time (with i3-4700, it takes about an hour).
```bash
make -j `getconf _NPROCESSORS_ONLN` deb-pkg LOCALVERSION=-rtai
```
Extract RTAI-patched kernel's image and headers.
```bash
cd /usr/src
```
```bash
dpkg -i linux-image-3.4.6-rtai_3.4.6-rtai-1_amd64.deb
```
```bash
dpkg -i linux-headers-3.4.6-rtai_3.4.6-rtai-1_amd64.deb
```
The bootloader should be automatically configured. Therefore, at this point, if we reboot, we can choose the RTAI kernel from Advanced Options.  
If the new kernel is not added to the bootloader list, use `grub-customizer`,  
```bash
add-apt-repository ppa:danielrichter2007/grub-customizer
apt-get update
apt-get install grub-customizer
```
Run grub-customizer,
In `General settings > default entry`, under predefined, choose  
```
Advanced options for Ubuntu>Ubuntu, with Linux 3.4.113-rt145
```
### 4. Install RTAI 
```bash
cd /usr/src/rtai-4.0
```
```bash
make menuconfig
```
1. Specify where RTAI is to be installed and where it should look for the patched kernel.  
Under `General > Installation directory` = /usr/realtime  
Under `General > Linux source tree` = /usr/src/linux-3.4.6
2. Choose `General > Inlining mode of user-space services > Eager inlining`
3. Under `Machine > Number of CPUs (SMP-only)` = 2 (for my CPU)
4. Disable `Add-ons > Real-Time COMEDI support in user space`
5. (Optional, and only required for RTAI LXRT) Enable `Add-one > Real-Time Driver Model over RTAI`

Compile and install RTAI,
```bash
make -j `getconf _NPROCESSORS_ONLN`
```
```bash
make install
```
### 5. Start the modules at startup  
RTAI programs (themselves kernel modules) communicate with core RTAI modules. Thus, these have to loaded prior to loading an RTAI program. 
To automatically load the modules before startup, add just before `do_start`,
```bash
nano /etc/init.d/rc.local
```
```bash
/sbin/insmod /usr/realtime/modules/rtai_hal.ko
/sbin/insmod /usr/realtime/modules/rtai_sched.ko
/sbin/insmod /usr/realtime/modules/rtai_fifos.ko
/sbin/insmod /usr/realtime/modules/rtai_sem.ko
/sbin/insmod /usr/realtime/modules/rtai_mbx.ko
/sbin/insmod /usr/realtime/modules/rtai_msg.ko
/sbin/insmod /usr/realtime/modules/rtai_netrpc.ko
/sbin/insmod /usr/realtime/modules/rtai_shm.ko
```
Additionally, in order to run RTAI userspace (LXRT) programs that use the Real Time Driver Model (RTDM), you should add,
```bash
/sbin/insmod /usr/realtime/modules/rtai_rtdm.ko
```
___
**Note:** [Only one RTAI scheduler exsits](https://mail.rtai.org/pipermail/rtai/2015-February/026696.html), namely `rtai_sched`. `rtai_lxrt.ko` is "linked to" (i.e. is a shortcut for) `rtai_sched.ko`.  
___
Now save the changes and exit.  
Reboot and then check if all the modules above are loaded,
```bash
lsmod | grep rtai
```
### 6. Run the latency test
To start the kernel space test:
```bash
cd /usr/realtime/testsuite/kern/latency
./run
```
To start the user space latency test:
```bash
cd /usr/realtime/testsuite/user/latency
./run
```
### 7. Install IgH EtherCAT Master
See [IgH EtherCAT Master installation guide](https://github.com/mohse-n/L7N_EtherLab/blob/master/Documentation/IgH%20EtherCAT%20Master%20Installation%20Guide.md).
### Reinstalling RTAI 
In case you needed to reinstall RTAI, delete the folder `realtime` in `/usr` and repeat this guide from step 4.
