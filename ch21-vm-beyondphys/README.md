# Overview

In this homework, you'll be investigating swap performance with a simple
program found in `mem.c`. The program is really simple: it just allocates
an array of integers of a certain size, and then proceeds to loop through
it (repeatedly), incrementing each value in the array.

Type `make` to build it (and look at the file `Makefile` for details
about how the build works).

Then, type `./mem` followed by a number to run it. The number is the size
(in MB) of the array. Thus, to run with a small array (size 1 MB):

```sh
prompt> ./mem 1
```

and to run with a larger array (size 1 GB):

```sh
prompt> ./mem 1024
```

The program prints out the time it takes to go through each loop as
well as the bandwidth (in MB/s). Bandwidth is particularly interesting
to know as it gives you a sense of how fast the system you're using can
move through data; on modern systems, this is likely in the GB/s range.

Here is what the output looks like for a typical run:

```sh
prompt> ./mem 1000
allocating 1048576000 bytes (1000.00 MB)
  number of integers in array: 262144000
loop 0 in 448.11 ms (bandwidth: 2231.61 MB/s)
loop 1 in 345.38 ms (bandwidth: 2895.38 MB/s)
loop 2 in 345.18 ms (bandwidth: 2897.07 MB/s)
loop 3 in 345.23 ms (bandwidth: 2896.61 MB/s)
^C
prompt>
```

The program first tells you how much memory it allocated (in bytes,
MB, and in the number of integers), and then starts looping through
the array. The first loop (in the example above) took 448 milliseconds;
because the program accessed the 1000 MB in just under half a second,
the computed bandwidth is (not surprisingly) just over 2000 MB/s.

The program continues by doing the same thing over and over, for loops 1,
2, etc.

Important: to stop the program, you must kill it. This task is
accomplished on Linux (and all Unix-based systems) by typing control-C
(^C) as shown above.

Note that when you run with small array sizes, each loop's performance
numbers won't be printed. For example:

```sh
prompt>  ./mem 1
allocating 1048576 bytes (1.00 MB)
  number of integers in array: 262144
loop 0 in 0.71 ms (bandwidth: 1414.61 MB/s)
loop 607 in 0.33 ms (bandwidth: 3039.35 MB/s)
loop 1215 in 0.33 ms (bandwidth: 3030.57 MB/s)
loop 1823 in 0.33 ms (bandwidth: 3039.35 MB/s)
^C
prompt>
```

In this case, the program only prints out a sample of outputs, so as
not to flood the screen with too much output.

The code itself is simple to understand. The first important part is a
memory allocation:

```c
    // the big memory allocation happens here
    int *x = malloc(size_in_bytes);
```

Then:

```c
    // Clever trick to force the very first print
    // in spite of the 0.2 threshold
    double time_since_last_print = 2.0;
```

By initializing `time_since_last_print = 2.0;`, the author guaranteed
that the `if` statement would evaluate to **`true`** the very first
time the program finished the first loop (Loop 0), regardless of how
fast that loop was.

**Why do this?**

1. **Immediate Feedback:** It lets the user know immediately that the
   program is working. If they had initialized it to `0.0`, and the loops
   were very slow (e.g., if you allocated 10GB of RAM), the user might
   be staring at a blank screen for several seconds before the first line
   appeared.
2. **Debugging:** It confirms the "Loop 0" stats (which are often the most
   interesting because of cache warming or demand paging) are always visible.

Then, the main loop begins:

```c
    while (1) {
	x[i++] += 1; // main work of loop done here.
```

The rest is just timing and printing out information. See `mem.c` for
details. Much of the homework revolves around using the tool `vmstat`
to monitor what is happening with the system. Read the `vmstat` man page
(type `man vmstat`) for details on how it works, and what each column
of output means.

---

## Improved version

Your instincts are correct. This program (a classic from the OSTEP
homework) is written to be a simple "memory spinner," but its reporting
logic is definitely "sloppy" and mathematically misleading.

Here is the breakdown of the two issues you identified:

### 1. The `loop_count` Increment (Logic Error)

You are right. In the original code, `loop_count` is only incremented
**after** the print check.

- **The Problem:** When it prints "loop 0", it has actually finished
  the first loop. When it prints "loop 1006", it has actually finished
  1007 loops.
- **The Fix:** `loop_count++` should happen immediately when `i ==
num_ints`. This ensures that the number printed on the line represents
  the loop that was _just completed_ and timed.

### 2. The Misleading Statistics (Conceptual Error)

This is the bigger issue. The current program prints the time and
bandwidth of **only the single most recent loop** that happened to cross
the 0.2s threshold.

- **The Problem:** It ignores the time and work done in all the loops
  that happened _between_ the prints.
- **Example:** If 1000 loops happen in 0.2 seconds, it discards the
  data for loops 1 through 999 and only tells you how fast loop 1000
  was. This creates "jittery" output because you are seeing a snapshot of
  one loop's luck with the OS scheduler, not the average performance over
  that 0.2s window.

### The Improved Version

To make this program scientifically useful, we should:

1. **Accumulate Work:** Count how many loops happened since the last
   print.
2. **Calculate Averages:** Divide the _total bytes moved_ during those
   0.2s by the _total time elapsed_.

I have rewritten the program below to fix the `loop_count` logic and
provide accurate, averaged bandwidth reporting.

### Key Improvements Made:

1. **Cumulative Bandwidth:** It now calculates `(Loops In Window * Size)
/ Time In Window`. This gives you a true average bandwidth that isn't
   fooled by a single slow loop.
2. **`atoll` vs `atoi`:** Used `atoll` for the input to prevent overflow
   if you ever try to allocate more than 2GB on a system where `int`
   is 32-bit.
3. **Memset:** Added a `memset` before starting. In Linux, `malloc`
   is "lazy"—it doesn't actually give you physical RAM until you touch
   it. Without the `memset`, the first loop is artificially slow because
   the OS is busy zeroing pages and updating page tables (demand paging).
4. **Formatting:** Added padding to the `printf` so the columns don't
   jump around as the numbers get bigger.

Now, when you run this, the "Bandwidth" will represent the **actual
sustained throughput** of your memory bus over that 0.2s period.

---

## `vmstat`

It is perfectly normal to feel overwhelmed. OSTEP covers the "Three
Easy Pieces" (Virtualization, Concurrency, Persistence), and they often
overlap in ways that feel like a "chicken and egg" problem.

Here is a breakdown of those `vmstat` fields translated into the concepts
you are learning in the Virtualization chapters.

### 1. Memory: The "Buff/Cache" vs. "Free" Puzzle

The memory columns are **complementary**. To the Kernel, "Free" RAM is
"Wasted" RAM.

- **`free`**: Truly empty RAM. The kernel tries to keep this small
  because it wants to use all available space to speed things up.
- **`buff` (Buffers)**: This is "Metadata" storage. It stores things like
  disk block locations, folder structures, and permission bits. Think of
  it as the **Table of Contents** for your disk.
- **`cache` (Page Cache)**: This is the **actual content** of files
  you’ve read recently. If you read a 1GB file, it stays in `cache`
  so that the second time you read it, it’s instant.
- **Are they complementary?** Yes. `free` + `buff` + `cache` + `used`
  (not shown directly in vmstat) **Total Physical RAM**.
- _Key OSTEP Concept:_ Memory Virtualization. If a program needs memory
  and `free` is low, the kernel will "evict" (delete) items from the
  `cache` to make room. **Cache and Buffers are "reclaimable."**

### 2. System: The "Administrative Tax"

This section measures how much the OS has to "intervene" to keep things
running.

- **`cs` (Context Switch)**: This is the heart of **Limited Direct
  Execution (LDE)**. To switch from Process A to Process B, the OS must:

1. Save A’s registers to its kernel stack.
2. Change the stack pointer to B’s kernel stack.
3. Restore B’s registers.

- _In your Exercise:_ When you ran 6 instances on 4 cores, `cs` exploded
  because the OS was constantly performing this "save/restore" dance to
  give everyone a fair slice.

- **`in` (Interrupts)**: A hardware signal that "interrupts" the CPU.
- _The Timer Interrupt:_ The most important one for you. The OS sets
  a timer (e.g., every 10ms). When it fires, an interrupt happens, the
  CPU jumps to the **Trap Table**, and the OS Scheduler gets control
  back. Without interrupts, a program like your `while(1)` loop would own
  the CPU forever, and you could never hit `Ctrl+C`.

### 3. CPU: The "Where is my time going?" Section

These are percentages that always add up to 100%.

- **`id` (Idle)**: The CPU is literally doing nothing. It’s executing
  a special "Halt" instruction to save power.
- **`wa` (Wait for I/O)**: The CPU is idle, **but it has a reason.**
  It is waiting for the SSD/HDD to return data.
- _Difference:_ If `id` is high, the system is bored. If `wa` is high,
  the system is **blocked** (the disk is too slow for the CPU). You saw
  this during your "8GB swap" experiment—the CPU was waiting for the
  slow SSD to swap pages.

- **`st` (Steal)**: Only happens in Virtual Machines (like AWS or WSL). It
  is time that your **Host** (Windows) took away from the **Guest** (Linux)
  to do something else. If you see `st`, it means your Windows 10 is busy
  and is "stealing" cycles from your Linux VM.
- **`gu` (Guest)**: The opposite of steal. It’s time the Linux kernel
  spent running a _sub-virtual machine_ (like a Docker container or KVM). In
  your current homework, this will likely stay at 0.

### Summary reminder for your OSTEP journey:

1. **Context Switches (`cs`)** are the mechanism of **Concurrency**.
2. **Interrupts (`in`)** are how the OS maintains **Control**.
3. **Swap (`si/so`)** is how the OS maintains the **Illusion** of
   infinite memory.
4. **`si` / `so`:** Physical Disk I/O (Swap-In/Out).
5. **`bi` / `bo`:** Block-In/Out (usually regular file I/O).
6. **Cache/Buffers** are how the OS hides the **Latency** of the disk.

If you see `si/so` (swapping) and `wa` (waiting) high at the same time,
your "Illusion" of infinite memory is breaking because your "Latency"
hiding (cache) has run out of space!
