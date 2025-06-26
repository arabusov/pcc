# PCC port for 16-bit MINIX (v1.x.x and v2.x.x)
It looks like the old MINIX used to be supported by PCC, but the current
upstream build is broken, and some source-code modifications are required.
Additionally, to be able to compile on MINIX 1.7.5 we better get rid of the
autotools scripts and use a Makefile only.

## File tree structure
  - `1983`: Pre-ANSI PCC sent by Al Kossow to
    [TUHS](https://www.tuhs.org/Archive/Applications/Portable_CC/), I took
    the 286 version of PCC done by MIT's Laboratory for Computer Science.
    - `c286`: All-in-one one-pass PCC compiler, target OS is VENIX.
    - `yacc`: BSD2.11 YACC, it produces slightly different `y.tab.c` then the
     original 1983 `yacc` used by MIT's Lab for CS.
  - `2023`: C99 PCC, this PCC port is based on the
    [SlackBuild](http://slackbuilds.org/repository/15.0/development/pcc/)
    variant.
    - `ccom`: The C compiler (`/usr/src/cmd/pcc` on UNIX V7
    - `common`: common utils for C/C++/F77.
    - `i86`: machine-dependent part, here: i286 only
    - `minix`: OS-dependent part, here, MINIX 1 and 2 only
    - `mip`: machine-independent part, in `/usr/src/cmd/mip` on UNIX V7
    - `tests`: directory with some compiler tests
    - `scripts`: developer scripts

## Status
The `2023` version is too big for the 16-bit `ack` compiler to translate
even the first C file. Thus, I'm going to use it as a reference for the
`1983`'s version. The one-pass version of the latter one just fits into 64k
separate I/D, which is amazing! However, it is not even an ANSI-C compiler,
so even MINIX's `/usr/include/stdio.h` causes a lot of compile errors.

## TODO
Priorities:
  1. Port `a286` and `l86` to MINIX,
  2. Ansify the `1983` version,
  3. `far` vs `near` pointers for protected mode.

