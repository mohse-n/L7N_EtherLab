## IgH EtherCAT Master Installation Guide: 

**Note:** Credit for this guide should be given to the [one](http://lists.etherlab.org/pipermail/etherlab-users/2015/002820.html) sent to EtherLab mailing list by Thomas Bitsky.  
---
Download the 1.5.2 tarball from [here](http://www.etherlab.org/en/ethercat/). 
Extract and move it to /usr/local/src. Here I'm assuming you have extracted it in the home directory.
```bash
sudo -s
```
```bash
sudo mv ethercat-1.5.2 /usr/local/src/
```
Just for convenience (typing "ethercat" instead of "ethercat-1.5.2"), create a symbolic link,
```bash
ln -s ethercat-1.5.2 ethercat
```
Move into the source directory,
```bash
cd ethercat
```
Configure the Makefile
**Note:** Be sure to apply the last configuration if have installed RTAI in /usr/realtime. If it's installed in a different directory, adjust the specified path accordingly. If you don't intend to compile RTAI codes at all, ignore this option.
**Note:** We're going to use the modified Realtek8169 driver. Hence, the "--enable-r8169" option.  
```bash
./configure --enable-generic -–disable-8139too --enable-r8169 --with-rtai-dir=/usr/realtime
```





