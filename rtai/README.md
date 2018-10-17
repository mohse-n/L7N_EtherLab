Basic structure of an EtherLab RTAI program:  
1. Definition and initialization of variables  
2. Definition of the cyclic task (run)  
3. Kernel module stuff: __init and __exit and module info  
---

See RTAI documentation (available online and in the tarball) and [IgH EtherCAT Master 1.1 Documentation](https://www.etherlab.org/download/ethercat/igh-ethercat-master-1.1.pdf) for comments on the RTAI API.

---

RTAI installation procedure is described in RTAI_Installation_Guide.md. 

---

To make the module, when your in /examples/rtai directory, run as superuser,
```bash
make modules
```
To load the module,
```bash
insmod ec_rtai_sample.ko
```
And to remove the module,
```bash
rmmod ec_rtai_sample.ko
```
**Note:** After is has been loaded, there is the possiblity of the module not exiting properly after it runs into a (minor) error (i.e. kernel oops). In such cases, the kenrel (computer) should be rebooted as there is no way of removing the module. 
