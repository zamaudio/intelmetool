intelmetool
===========

Compiling:
==========

You need kernel headers installed for your distro.

Running:
========

You need to `sudo rmmod mei_me` and `sudo rmmod mei` before using this kernel module.

Interesting to compare output with and without partially removing ME with
`me_cleaner`.
(NB: Always back up your BIOS when fiddling with ME firmware)

```
make
sudo insmod ./notme.ko
dmesg
```

You should expect the module to error out with the following message:
```
insmod: ERROR: could not insert module ./notme.ko: No such device
```

But in the kernel message log, dmesg should show the correct output:
```
[ 3648.666895] notme: === Damagement Engine driver loaded ===
[ 3648.666936] notme: probe: Found Damagement Engine
[ 3648.666937] notme: MEI found: [8086:1c3a]
[ 3648.666943] notme: ME Status   : 0x1e003052
[ 3648.666950] notme: ME Status 2 : 0x16320002
[ 3648.666951] notme: FW Partition Table      : OK
[ 3648.666952] notme: Bringup Loader Failure  : NO
[ 3648.666953] notme: Firmware Init Complete  : NO
[ 3648.666954] notme: Manufacturing Mode      : YES
[ 3648.666954] notme: Boot Options Present    : NO
[ 3648.666955] notme: Update In Progress      : NO
[ 3648.666956] notme: Current Working State   : Recovery
[ 3648.666957] notme: Current Operation State : M0 with UMA
[ 3648.666958] notme: Current Operation Mode  : Normal
[ 3648.666959] notme: Error Code              : Image Failure
[ 3648.666960] notme: Progress Phase          : BUP Phase
[ 3648.666961] notme: Power Management Event  : Pseudo-global reset
[ 3648.666962] notme: (BUP) Progress Phase State    : M0 kernel load
[ 3648.666963] notme: ME is crippled on this board
[ 3648.666999] notme: === Damagement Engine driver unloaded ===
```
