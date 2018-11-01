
## PREEMPT_RT Installtion Guide

### 1. Decide on a kernel version
There are two versions to take into account when detemining the kernel version:  
* **Igh EtherCAT Master:** The package has modified network card drivers only for specific versions of the kernel. 
* **PREEMPT_RT**: Kernel patches are available for some kernels and not others.
#### Igh EtherCAT Master
Since we're going to write IgH EtherCAT Master (from here on called IgH Master) programs, we consider its version first and proceed accordingly. 
The current IgH EtherCAT version is 1.5.2 and can be downloaded from the [website](https://www.etherlab.org/en/ethercat/index.php). 
___
**Note:** The [SourceForge repository](https://sourceforge.net/projects/etherlabmaster) is regularly updated and is much more recent, but during this walkthrough, we stick to the tried-and-tested versions due to better stability and support from the community.  
___
Looking at the "devices" folder, we can see the modified (and original) drivers and their associated kernel versions.
For instance,  
```
r8169-3.4-ethercat.c  
``` 
is the modified driver for Realtek8169 family of network cards for kernel 3.4.x . Keep 3.4 in mind and move on to the next section (RTAI).  
___
**Note:** Drivers for the more recent kernels are available on the [SourceForge repository](https://sourceforge.net/projects/etherlabmaster) and Gavin Lambert's [unofficial patchset](https://sourceforge.net/u/uecasm/etherlab-patches/ci/default/tree/#readme).  
___
#### PREEMPT_RT
Now we should look for a version of 3.4.x kernel for which a PREEMPT_RT patch is available. The existence of .patch file should be checked by visiting   
```
https://mirrors.edge.kernel.org/pub/linux/kernel/projects/rt/
``` 
In '/3.4' directory, the only available patch is that of kernel 3.4.113.
```
patch-3.4.113-rt145.patch.xz   
```
Therefore, we choose 3.4.113 as our kernel.
### 2. Download the required files
```bash
sudo -s
```
We use cURL for downloading RTAI and Linux kernel.
```bash
apt-get install curl
```
Kernels have to exist at '/usr/src', and we're going to download everything in that directory.
```bash
cd /usr/src
```
Download the patch,
```bash
curl -L https://mirrors.edge.kernel.org/pub/linux/kernel/projects/rt/3.4/patch-3.4.113-rt145.patch.xz | tar xj
```
Download the associated kernel,
```bash
curl -L https://www.kernel.org/pub/linux/kernel/v3.x/linux-3.4.113.tar.xz | tar xJ
```
We are going start from the default kernel configuration when building the PREEMPT-patched kernel. This way we'll be sure that the only cause of possbile failure or improvements are **our** modifications.
___
**Note:** Entering the right URL below requires a visit to http://kernel.ubuntu.com/~kernel-ppa/mainline.
___
```bash
curl -L http://kernel.ubuntu.com/~kernel-ppa/mainline/v3.4.113/linux-image-3.4.113-0304113-generic_3.4.113-0304113.201610261546_amd64.deb -o linux-image-3.4.113-generic-amd64.deb
```
Extract the .deb package. Unfortunately, dpkg doesn't support multithreading, so this and similar steps might take longer than you expect.
```bash
dpkg-deb -x linux-image-3.4.113-generic-amd64.deb linux-image-3.4.113-generic-amd64
```
Install the dependencies. I'm not certain if this is the minimal set of dependencies and perhaps some could be removed, but these worked for me.
```bash
apt-get update
```
```bash
apt-get install cvs subversion build-essential git-core g++-multilib gcc-multilib
```
```bash
apt-get install libtool automake libncurses5-dev kernel-package
```
```bash
apt-get install docbook-xsl fop libncurses5 libpcre3 libpvm3 libquadmath0 libstdc++6 libtinfo5 libxml2 tcl8.5 tk8.5 zlib1g libgcc1 libc6 libblas-dev gfortran liblapack-dev libssl-dev portaudio19-dev portaudio19-doc
```

### 3. Patch, configure, and build the kernel
Replace the default .config file with the configuration file of the associated Ubuntu kernel,
```bash
cp /usr/src/linux-image-3.4.113-generic-amd64/boot/config-3.4.6-030406-generic /usr/src/linux-3.4.113/.config
```
Apply the PREEMPT_RT patch to the kernel source files,
```bash
cd /usr/src/linux-3.4.113
```
```bash
patch -p1 < /usr/src/patch-3.4.113-rt145.patch
```
Now we're ready to configure the kernel.
```bash
make menuconfig
```
1. If you're using a 64-bit CPU: "Processor type and features > Processor family > Generic x86_64"
2. Number of physical cores (i.e. don't account for hyperthreading): "Processor type and features > Maximum number of CPU’s > 2" (My PC had i3-4700, which has 2 physical cores)
3. Disable “Processor type and features” > SMT (Hyperthreading) scheduler support”
4. Enable “Processor type and features > Symmetric multi-processing support"
5. Under “Power management and ACPI options”, disable anything that you can, including "CPU Frequency Scaling" and "CPU idle PM support".
6. Under "Power management and ACPI options > ACPI", disable everything you're able to, except “Power Management Timer Support” and "Button".  
7. High res timer  
8. Select "Exit" and save.  
___
**Note:** Also worth checking are the various guides and recommendations for the optimal kernel configuration in linuxcnc website and forum.
___
Now we should be able to compile the kernel. Note that since many, many device drivers are enabled and will be compiled, the installation process takes a significant amount of time (with i3-4700, it takes about an hour).
```bash
make -j `getconf _NPROCESSORS_ONLN` deb-pkg 
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
The bootloader should be automatically configured. Therefore, at this point, if we reboot, we can choose the rt kernel from Advanced Options.
### 4. Run the latency test
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
### 5. Install IgH EtherCAT Master
See [IgH EtherCAT Master installation guide](https://github.com/mohse-n/L7N_EtherLab/blob/master/Installation%20guides/IgH%20Master%20Installation%20Guide.md).
### Reinstalling the kernel 
???
