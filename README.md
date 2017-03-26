intelmetool
===========

Compiling:
==========

You need libpci-dev

Running:
========

You need to `sudo rmmod mei_me` and `sudo rmmod mei` before using this tool.
Also pass `iomem=relaxed` to kernel commandline if you are on kernel 4.4+

Interesting to compare output with and without partially removing ME with
`me_cleaner`.
(NB: Always back up your BIOS when fiddling with ME firmware)

```
make
sudo ./intelmetool
```
