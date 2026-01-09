## Homework

Each chapter has some questions at the end; we call these "homework",
because you should do the "work" at your "home". Make sense? It's one of
the innovations of this book.

Homework can be used to solidify your knowledge of the material in each
of the chapters. Many homework are based on running a simulator, which
mimic some aspect of an operating system. For example, a disk scheduling
simulator could be useful in understanding how different disk scheduling
algorithms work. Some other homework are just short programming exercises,
allowing you to explore how real systems work.

For the simulators, the basic idea is simple: each of the simulators below
let you both generate problems and obtain solutions for an infinite number
of problems. Different random seeds can usually be used to generate different
problems; using the `-c` flag computes the answers for you (presumably after
you have tried to compute them yourself!).

Each homework included below has a README file that explains what to
do. Previously, this material had been included in the chapters themselves,
but that was making the book too long. Now, all that is left in the book
are the questions you might want to answer with the simulator; the details
on how to run code are all in the README.

Thus, your task: read a chapter, look at the questions at the end of the
chapter, and try to answer them by doing the homework. Some require a
simulator (written in Python); those are available by below. Some others
require you to write some code. At this point, reading the relevant README
is a good idea. Still others require some other stuff, like writing C code
to accomplish some task.

To use these, the best thing to do is to clone the homework. For example:

```sh
prompt> git clone https://github.com/remzi-arpacidusseau/ostep-homework/
prompt> cd file-disks
prompt> ./disk.py -h
```

### Roadmap

## Introduction

| Chapter                                                         | What To Do        |
| --------------------------------------------------------------- | ----------------- |
| [2 Introduction](http://www.cs.wisc.edu/~remzi/OSTEP/intro.pdf) | No homework (yet) |

## Virtualization

| Chapter                                                                                 | What To Do                                                |
| --------------------------------------------------------------------------------------- | --------------------------------------------------------- |
| [4 Abstraction: Processes](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf)           | Run [process-run.py](ch04-cpu-intro)                      |
| [5 Process API](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf)                        | Run [fork.py](ch05-cpu-api) and write some code           |
| [6 Direct Execution](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf)            | Write some code                                           |
| [7 Scheduling Basics](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-sched.pdf)                | Run [scheduler.py](ch07-cpu-sched)                        |
| [8 MLFQ Scheduling](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-sched-mlfq.pdf)             | Run [mlfq.py](ch08-cpu-sched-mlfq)                        |
| [9 Lottery Scheduling](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf)       | Run [lottery.py](ch09-cpu-sched-lottery)                  |
| [10 Multiprocessor Scheduling](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-sched-multi.pdf) | Run [multi.py](ch10-cpu-sched-multi)                      |
| [13 Abstraction: Address Spaces](http://www.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf)      | Write some code                                           |
| [14 VM API](http://www.cs.wisc.edu/~remzi/OSTEP/vm-api.pdf)                             | Write some code                                           |
| [15 Relocation](http://www.cs.wisc.edu/~remzi/OSTEP/vm-mechanism.pdf)                   | Run [relocation.py](ch15-vm-mechanism)                    |
| [16 Segmentation](http://www.cs.wisc.edu/~remzi/OSTEP/vm-segmentation.pdf)              | Run [segmentation.py](ch16-vm-segmentation)               |
| [17 Free Space](http://www.cs.wisc.edu/~remzi/OSTEP/vm-freespace.pdf)                   | Run [malloc.py](ch17-vm-freespace)                        |
| [18 Paging](http://www.cs.wisc.edu/~remzi/OSTEP/vm-paging.pdf)                          | Run [paging-linear-translate.py](ch18-vm-paging)          |
| [19 TLBs](http://www.cs.wisc.edu/~remzi/OSTEP/vm-tlbs.pdf)                              | Write some code                                           |
| [20 Multi-level Paging](http://www.cs.wisc.edu/~remzi/OSTEP/vm-smalltables.pdf)         | Run [paging-multilevel-translate.py](ch20-vm-smalltables) |
| [21 Paging Mechanism](http://www.cs.wisc.edu/~remzi/OSTEP/vm-beyondphys.pdf)            | Run [mem.c](ch21-vm-beyondphys)                           |
| [22 Paging Policy](http://www.cs.wisc.edu/~remzi/OSTEP/vm-beyondphys-policy.pdf)        | Run [paging-policy.py](ch22-vm-beyondphys-policy)         |
| [23 Complete VM](http://www.cs.wisc.edu/~remzi/OSTEP/vm-complete.pdf)                   | No homework (yet)                                         |

## Concurrency

| Chapter                                                                              | What To Do                                    |
| ------------------------------------------------------------------------------------ | --------------------------------------------- |
| [26 Threads Intro](http://www.cs.wisc.edu/~remzi/OSTEP/threads-intro.pdf)            | Run [x86.py](ch26-threads-intro)              |
| [27 Thread API](http://www.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf)                 | Run [some C code](ch27-threads-api)           |
| [28 Locks](http://www.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf)                    | Run [x86.py](ch28-threads-locks)              |
| [29 Lock Usage](http://www.cs.wisc.edu/~remzi/OSTEP/threads-locks-usage.pdf)         | Write some code                               |
| [30 Condition Variables](http://www.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf)         | Run [some C code](ch30-threads-cv)            |
| [31 Semaphores](http://www.cs.wisc.edu/~remzi/OSTEP/threads-sema.pdf)                | Read and write [some code](ch31-threads-sema) |
| [32 Concurrency Bugs](http://www.cs.wisc.edu/~remzi/OSTEP/threads-bugs.pdf)          | Run [some C code](ch32-threads-bugs)          |
| [33 Event-based Concurrency](http://www.cs.wisc.edu/~remzi/OSTEP/threads-events.pdf) | Write some code                               |

## Persistence

| Chapter                                                                                        | What To Do                                                 |
| ---------------------------------------------------------------------------------------------- | ---------------------------------------------------------- |
| [36 I/O Devices](http://www.cs.wisc.edu/~remzi/OSTEP/file-devices.pdf)                         | No homework (yet)                                          |
| [37 Hard Disk Drives](http://www.cs.wisc.edu/~remzi/OSTEP/file-disks.pdf)                      | Run [disk.py](ch37-file-disks)                             |
| [38 RAID](http://www.cs.wisc.edu/~remzi/OSTEP/file-raid.pdf)                                   | Run [raid.py](ch38-file-raid)                              |
| [39 FS Intro](http://www.cs.wisc.edu/~remzi/OSTEP/file-intro.pdf)                              | Write some code                                            |
| [40 FS Implementation](http://www.cs.wisc.edu/~remzi/OSTEP/file-implementation.pdf)            | Run [vsfs.py](ch40-file-implementation)                    |
| [41 Fast File System](http://www.cs.wisc.edu/~remzi/OSTEP/file-ffs.pdf)                        | Run [ffs.py](ch41-file-ffs)                                |
| [42 Crash Consistency and Journaling](http://www.cs.wisc.edu/~remzi/OSTEP/file-journaling.pdf) | Run [fsck.py](ch42-file-journaling)                        |
| [43 Log-Structured File Systems](http://www.cs.wisc.edu/~remzi/OSTEP/file-lfs.pdf)             | Run [lfs.py](ch43-file-lfs)                                |
| [44 Solid-State Disk Drives](http://www.cs.wisc.edu/~remzi/OSTEP/file-ssd.pdf)                 | Run [ssd.py](ch44-file-ssd)                                |
| [45 Data Integrity](http://www.cs.wisc.edu/~remzi/OSTEP/file-integrity.pdf)                    | Run [checksum.py](ch45-file-integrity) and Write some code |
| [48 Distributed Intro](http://www.cs.wisc.edu/~remzi/OSTEP/dist-intro.pdf)                     | Write some code                                            |
| [49 NFS](http://www.cs.wisc.edu/~remzi/OSTEP/dist-nfs.pdf)                                     | Write some analysis code                                   |
| [50 AFS](http://www.cs.wisc.edu/~remzi/OSTEP/dist-afs.pdf)                                     | Run [afs.py](ch50-dist-afs)                                |

## Follow up

```sh
.
├── ch04-cpu-intro              (done)
├── ch05-cpu-api                (done)
├── ch06-ltd-direct-execution   (done)
├── ch07-cpu-sched              (done)
├── ch08-cpu-sched-mlfq         (done)
├── ch09-cpu-sched-lottery      (done)
├── ch10-cpu-sched-multi        (done)
├── ch13-address-spaces         (done)
├── ch14-memory-api             (done)
├── ch15-vm-mechanism           (done)
├── ch16-vm-segmentation        (done)
├── ch17-vm-freespace           (done)
├── ch18-vm-paging              (done)
├── ch20-vm-smalltables         (done)
├── ch21-vm-beyondphys
├── ch22-vm-beyondphys-policy
├── ch26-threads-intro
├── ch27-threads-api
├── ch28-threads-locks
├── ch30-threads-cv
├── ch31-threads-sema
├── ch32-threads-bugs
├── ch36-file-devices
├── ch37-file-disks
├── ch38-file-raid
├── ch40-file-implementation
├── ch41-file-ffs
├── ch42-file-journaling
├── ch43-file-lfs
├── ch44-file-ssd
├── ch45-file-integrity
└── ch50-dist-afs
```
