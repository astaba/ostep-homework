# Overview

This fun little homework tests if you understand how a multi-level page
table works. And yes, there is some debate over the use of the term fun in
the previous sentence. The program is called: `paging-multilevel-translate.py`

Some basic assumptions:

- The page size is an unrealistically-small 32 bytes
- The virtual address space for the process in question (assume there is
  only one) is 1024 pages, or 32 KB
- physical memory consists of 128 pages

Thus, a virtual address needs 15 bits (5 for the offset, 10 for the VPN). A
physical address requires 12 bits (5 offset, 7 for the PFN).

The system assumes a multi-level page table. Thus, the upper five bits of a
virtual address are used to index into a page directory; the page directory
entry (PDE), if valid, points to a page of the page table. Each page table
page holds 32 page-table entries (PTEs). Each PTE, if valid, holds the desired
translation (physical frame number, or PFN) of the virtual page in question.

The format of a PTE is thus:

```sh
  VALID | PFN6 ... PFN0
```

and is thus 8 bits or 1 byte.

The format of a PDE is essentially identical:

```sh
  VALID | PT6 ... PT0
```

You are given two pieces of information to begin with.

First, you are given the value of the page directory base register (PDBR),
which tells you which page the page directory is located upon.

Second, you are given a complete dump of each page of memory. A page dump
looks like this:

```sh
    page 0: 08 00 01 15 11 1d 1d 1c 01 17 15 14 16 1b 13 0b ...
    page 1: 19 05 1e 13 02 16 1e 0c 15 09 06 16 00 19 10 03 ...
    page 2: 1d 07 11 1b 12 05 07 1e 09 1a 18 17 16 18 1a 01 ...
    ...
```

which shows the 32 bytes found on pages 0, 1, 2, and so forth. The first byte
(0th byte) on page 0 has the value 0x08, the second is 0x00, the third 0x01,
and so forth.

You are then given a list of virtual addresses to translate.

Use the PDBR to find the relevant page table entries for this virtual
page. Then find if it is valid. If so, use the translation to form a final
physical address. Using this address, you can find the VALUE that the memory
reference is looking for.

Of course, the virtual address may not be valid and thus generate a fault.

Some useful options:

```sh
  -s SEED, --seed=SEED       the random seed
  -n NUM, --addresses=NUM    number of virtual addresses to generate
  -c, --solve                compute answers for me
```

Change the seed to get different problems, as always.

Change the number of virtual addresses generated to do more translations
for a given memory dump.

Use -c (or --solve) to show the solutions.

Good luck with this monstrosity!

## References

[BOH10] "Computer Systems: A Programmer’s Perspective" by Randal E. Bryant
and David R. O’Hallaron. Addison-Wesley, 2010. _We have yet to find a good
first reference to the multi-level page table. However, this great textbook
by Bryant and O’Hallaron dives into the details of x86, which at least is an
early system that used such structures. It’s also just a great book to have._

[JM98] "Virtual Memory: Issues of Implementation" by Bruce Jacob, Trevor
Mudge. IEEE Computer, June 1998. _An excellent survey of a number of different
systems and their approach to virtualizing memory. Plenty of details on x86,
PowerPC, MIPS, and other architectures._

[LL82] "Virtual Memory Management in the VAX/VMS Operating System" by Hank
Levy, P. Lipman. IEEE Computer, Vol. 15, No. 3, March 1982. _A terrific
paper about a real virtual memory manager in a classic operating system,
VMS. So terrific, in fact, that we’ll use it to review everything we’ve
learned about virtual memory thus far a few chapters from now._

[M28] "Reese’s Peanut Butter Cups" by Mars Candy Corporation. Published at
stores near you. _Apparently these fine confections were invented in 1928 by
Harry Burnett Reese, a former dairy farmer and shipping foreman for one Milton
S. Hershey. At least, that is what it says on Wikipedia. If true, Hershey and
Reese probably hate each other’s guts, as any two chocolate barons should._

[N+02] "Practical, Transparent Operating System Support for Superpages" by
Juan Navarro, Sitaram Iyer, Peter Druschel, Alan Cox. OSDI ’02, Boston,
Massachusetts, October 2002. _A nice paper showing all the details you have
to get right to incorporate large pages, or superpages, into a modern OS. Not
as easy as you might think, alas._

[M07] "Multics: History" Available:
http://www.multicians.org/history.html. _This amazing web site provides a huge
amount of history on the Multics system, certainly one of the most influential
systems in OS history. The quote from therein: "Jack Dennis of MIT contributed
influential architectural ideas to the beginning of Multics, especially the
idea of combining paging and segmentation." (from Section 1.2.1)_
