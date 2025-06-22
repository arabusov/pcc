# PCC port for 16-bit MINIX (v1.x.x and v2.x.x)
It looks like the old MINIX used to be supported by PCC, but the current
upstream build is broken, and some source-code modifications are required.
Additionally, to be able to compile on MINIX 1.7.5 we better get rid of the
autotools scripts and use a Makefile only.

This PCC port is based on the
[SlackBuild](http://slackbuilds.org/repository/15.0/development/pcc/)
variant maintained by urchlay.

## File tree structure
  - `ccom`: The C compiler (`/usr/src/cmd/pcc` on UNIX V7
  - `common`: common utils for C/C++/F77.
  - `i86`: machine-dependent part, here: i286 only
  - `minix`: OS-dependent part, here, MINIX 1 and 2 only
  - `mip`: machine-independent part, in `/usr/src/cmd/mip` on UNIX V7
  - `tests`: directory with some compiler tests
  - `scripts`: developer scripts
