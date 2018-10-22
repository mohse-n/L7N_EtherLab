## IgH EtherCAT Master Installation Guide:   
**Note:** This guide is basically [the one by Thomas Bitsky](http://lists.etherlab.org/pipermail/etherlab-users/2015/002820.html), with some comments added and some removed.  
___
**Note:** The installation sequence is   
1. kernel-RTAI    
2. RTAI   
3. IgH EtherCAT Master 

This guide describes the last step in the process.
___
### 1. Download the library
Download the snapshot of 1.5-stable branch from [here](https://sourceforge.net/p/etherlabmaster/code/ci/stable-1.5/tree/). I got the snapshot at commit 336936.
Extract and rename the folder to ethercat-1.5.2.
Move the folder to /usr/local/src. Here I'm assuming you have extracted it in the home directory.
```bash
sudo -s
```
```bash
mv ethercat-1.5.2 /usr/local/src/
```
Just for convenience (typing "ethercat" instead of "ethercat-1.5.2"), create a symbolic link,
```bash
cd /usr/local/src
```
```bash
ln -s ethercat-1.5.2 ethercat
```
Move into the source directory,
```bash
cd ethercat
```
### 2. Install the library
Make the bootstrap file executable and generate the configure file.
```bash
chmod +x bootstrap
```
```bash
./bootstrap
```
Make the configure file executable.
```bash
chmod +x configure
```
Configure the Makefile.
___
**Note:** We're going to use the modified Realtek8169 driver. Hence, the "--enable-r8169" option.   
___
**Note:** We're also going to write LXRT programs (RTAI in user space).Therefore, "--enable-rtdm".  
___
**Note:** Be sure to apply the last option if have installed RTAI in /usr/realtime. If it's installed in a different directory, adjust the specified path accordingly. If you don't intend to compile RTAI codes at all, ignore this option.   
___
**Note:** See IgH EtherCAT Master 1.5.2 documentation for other options.
___
```bash
./configure --enable-generic -–disable-8139too --enable-r8169 --enable-cycles --enable-rtdm --with-rtai-dir=/usr/realtime
```
Compile and install the modules,  
```bash
make
```
```bash
make modules
```
```bash
make install
```
```bash
make modules_install
```
```bash
depmod
```
### 3. Configure the master
There are some parameters associated with the master module (e.g. NIC MAC address, driver to use). We specify these in a file we put in /etc/sysconfig. 
```bash
sudo mkdir /etc/sysconfig/
```
```bash
sudo cp /opt/etherlab/etc/sysconfig/ethercat /etc/sysconfig/
```
```bash
sudo nano /etc/sysconfig/ethercat
```
Run "ifconfig" (in another terminal) and copy the MAC address of eth0. Paste it between the quotes in front of MASTER0_DEVICE.  
```bash
MASTER0_DEVICE="00:05:9a:3c:7a:00"
```
The field in front of DEVICE_MODULES is the name of the driver which the master modules will use. Since we compiled with --enable-r8169, we will use the modified r8169 driver for the best performance. 
```bash
DEVICE_MODULES=“r8619"
```
___
**Note:** If your network card isn't supported by EtherLab, or EtherLab doesn't support your kernel version, DEVICE_MODULES=“generic".  
___
Copy the initilization script,
```bash
cd /opt/etherlab
```
```bash
cp ./etc/init.d/ethercat /etc/init.d/
```
Give execution permission to all groups,
```bash
sudo chmod a+x /etc/init.d/ethercat
```
Make the ethercat command line tool available,
```bash
sudo ln -s /opt/etherlab/bin/ethercat /usr/local/bin/ethercat
```
The ethercat command line tool communicates with the master module as a character device, and we're going to give "normal" (non-root) users access to this device (thus enabling users to use command line tools).
Create 99-EtherCAT.rules file,
```bash
nano /etc/udev/rules.d/99-EtherCAT.rules
```
And add the following line to it,
```bash
KERNEL=="EtherCAT[0-9]*", MODE="0664"
```
At this point, you should be able to start the master,
```bash
sudo /etc/init.d/ethercat start
```
### Extra notes
___
**Note:** To view the kernel log, either open /var/log/syslog with a text editor or
```bash
tail -f -n 10 /var/log/syslog
```
To view the 10 latest kernel messages (printed with printk). Additionally,
```bash
dmesg
```
also outputs the last kernel messages.
___
**Note:** In case you needed to reinstall the package, delete "ethercat" and "ethercat-1.5.2" in /usr/local/src and follow the installation guide up to (and including) the "depmod" step. 
___
**Note:** To enable debug messages (in kernel log),
```bash
ethercat debug 1
```








