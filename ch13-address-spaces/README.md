# Monitor memory

## The `free` utility

> **Question:** What is the point of making the pages dirty before starting
timer and while loop since those pages will get dirty in the while loop anyway?

That is another excellent, very insightful question\! You've hit on the core
difference between **allocation** and **commitment** in memory management.

The initial loop (Lines 57-59) is **critical** because of how the C standard
library function `malloc()` interacts with the Linux kernel's virtual
memory system.

Here is the breakdown of why that first "dirtying" loop is necessary, even
though the `while` loop runs afterward:

### 1\. `malloc()` is "Lazy"

When you call `malloc(bytes_to_allocate)`:

  * **Virtual Memory:** The C library (using system calls like `mmap` or
  `brk`) reserves a large contiguous chunk of **virtual address space**
  for your program. The kernel updates the program's page tables to say,
  "If the program ever accesses this range of addresses, I'll figure out
  where to put it in physical RAM."
  * **No Physical Memory Commitment:** Crucially, `malloc` does **not**
  immediately commit the physical RAM (RAM pages) required to back that
  virtual space. It's lazy. At this stage, the `free` command's `used` and
  `available` metrics often don't reflect the new allocation because the
  kernel knows the memory hasn't actually been touched yet.

### 2\. The Purpose of the First Loop (Lines 57-59)

```c
// 3. Mark all pages as "dirty" initially
for (size_t i = 0; i < bytes_to_allocate; i += PAGE_SIZE) {
    memory_array[i] = 0xAA; // Write a distinct value
}
```

The sole purpose of this initial loop is to **force a page fault** for every
4KB memory page we allocated.

  * When your program attempts to write a byte (`memory_array[i] = 0xAA`),
  the kernel detects that this virtual address doesn't yet have a physical
  page associated with it.
  * The kernel then stops your program, finds a **free physical RAM page**,
  maps it to that virtual address, initializes it (usually to zero), and
  allows the write operation to complete. This is the moment the memory
  is **committed**.

This ensures that *before* the main timing loop starts, the full amount
of memory requested by the user is **actually backed by physical RAM**
and accurately reflected in the output of the `free` command.

### 3\. The Purpose of the Main `while` Loop (Lines 66-73)

```c
// 4. Continuous Streaming Loop
while (run_time_seconds < 0 || time(NULL) < end_time) {
    for (size_t i = 0; i < bytes_to_allocate; i += PAGE_SIZE) {
        *(volatile char *)&memory_array[i] = (char)access_counter;
    }
    // ...
}
```

This main loop serves a different, ongoing purpose: to **keep the memory
active** and prevent the kernel from reclaiming it.

  * If the program stopped touching the memory, the kernel's memory management
  algorithms might decide those pages are old and move them out to **swap
  space** or reclaim them for the **page cache** if memory pressure is high.
  * By continuously writing to these pages, we keep them **hot** and **dirty**,
  sending a strong signal to the kernel that this memory is actively being
  used and cannot be reclaimed or swapped out.

In summary, the **initial loop** gets the memory **committed** and counted
as used, and the **main loop** keeps that memory **committed** and prevents
it from being silently stolen or swapped out by the OS.

## `volatile` type qualifier

> **Question:** why would the compiler optimize away the `memory_array[i]`
memory write without the `volatile` type qualifier?

That's an extremely critical question about optimization in C, and it gets
to the heart of why low-level memory operations need explicit instructions
for the compiler.

The use of **`volatile`** in the `memory-user.c` program is not about the
operating system; it's a direct instruction to the **C compiler** to stop
it from performing an optimization that would destroy the intended behavior
of the program.

-----

### 1\. Why the Compiler Optimizes Memory Accesses

Modern compilers (like GCC or Clang) are highly aggressive at optimizing
code to run faster. If a variable or memory location is written to, and
then later read, the compiler may assume that no other process, thread,
or hardware device can change that memory in the meantime.

In your memory streaming loop:

```c
for (size_t i = 0; i < bytes_to_allocate; i += PAGE_SIZE) {
        // Write (touch) the memory location
        memory_array[i] = (char)access_counter;
}
```

The compiler sees that the same address (`memory_array[i]`) is being written
to repeatedly inside a tight loop. If the compiler were to aggressively
optimize this, it might decide:

1.  "I'm writing the value `access_counter` to `memory_array[i]`."
2.  "The next time I iterate, I'm just going to write a slightly different
value to the same location."
3.  **The Optimization:** The compiler might decide that repeatedly storing
the value into the main memory is slow. Instead, it might keep the value in
a **CPU register** (a tiny, lightning-fast memory location inside the CPU)
and only write it to the actual RAM location once, *after* the loop is
completely finished, or if it sees an external read.

**The Problem: Loop Elimination**

  In the worst case, the compiler might realize the loop's output is not
  externally visible (i.e., nothing is reading it *during* the loop) and
  **optimize the entire inner loop away**, effectively turning the code into
  a no-op that uses zero memory and defeats the whole purpose of the program.

-----

### 2\. What `volatile` Does

The keyword **`volatile`** stands for **"volatile storage class specifier."**

It is a promise you make to the compiler that the memory location associated
with a variable might be changed by something **outside the normal control
flow of the C program**.

When applied to a memory access, `volatile` forces the compiler to treat
every instruction involving that memory location as a necessary action.

**How `volatile` Works in Your Code**

  In the streaming loop:

  ```c
  *(volatile char *)&memory_array[i] = (char)access_counter;
  ```

  1.  **`&memory_array[i]`**: Gets the address of the memory location.
  2.  **`(volatile char *)`**: Casts that address to a pointer to a
  volatile `char`. This tells the compiler: **"The memory at this address
  is volatile."**
  3.  **`*`**: Dereferences the volatile pointer to perform the write.

  **The Compiler's New Behavior:**

  * **No Caching in Registers:** The compiler can no longer cache the memory
  address's value in a fast register across loop iterations.
  * **No Code Reordering/Removal:** The compiler cannot remove the write
  instruction, nor can it reorder it relative to other volatile accesses. It
  **must** generate an explicit, physical store operation to the memory
  address for *every single iteration* of the loop.

  This guarantee of an actual physical write to RAM is exactly what we need
  to ensure the operating system sees the memory as **actively used** and
  keeps the memory pages **committed** and **dirty**, preventing them from
  being swapped out or reclaimed

## The `pmap` utility

After running:

```bash
$ ./memory-user.out 1024
```

That is the full, verbose output from `pmap -x`, which provides the key
metrics for analyzing real memory usage: **Resident Set Size (RSS)** and
**Dirty Pages**. This output is perfect for studying the actual impact of
your memory-user program.

```bash
$ pmap 531915 -x
531915:   ./memory-user.out 1024
Address           Kbytes     RSS   Dirty Mode  Mapping
00005c7b25623000       4       4       0 r---- memory-user.out
00005c7b25624000       4       4       0 r-x-- memory-user.out
00005c7b25625000       4       4       0 r---- memory-user.out
00005c7b25626000       4       4       4 r---- memory-user.out
00005c7b25627000       4       4       4 rw--- memory-user.out
00005c7b422ea000     132       4       4 rw---   [ anon ]
00007042fabff000 1048580 1048576 1048576 rw---   [ anon ]
000070433ac00000     160     156       0 r---- libc.so.6
000070433ac28000    1568     980       0 r-x-- libc.so.6
000070433adb0000     316     100       0 r---- libc.so.6
000070433adff000      16      16      16 r---- libc.so.6
000070433ae03000       8       8       8 rw--- libc.so.6
000070433ae05000      64      28      28 rw---   [ anon ]
000070433ae23000       8       4       4 rw---   [ anon ]
000070433ae25000       4       4       0 r---- ld-linux-x86-64.so.2
000070433ae26000     172     168       0 r-x-- ld-linux-x86-64.so.2
000070433ae51000      40      40       0 r---- ld-linux-x86-64.so.2
000070433ae5b000       8       8       8 r---- ld-linux-x86-64.so.2
000070433ae5d000       8       8       8 rw--- ld-linux-x86-64.so.2
00007ffc13b5b000     136      20      20 rw---   [ stack ]
00007ffc13be9000      16       0       0 r----   [ anon ]
00007ffc13bed000       8       4       0 r-x--   [ anon ]
---------------- ------- ------- -------
total kB         1051264 1050144 1048680

```

Here is a structural summary and analysis of your `pmap -x` output, integrating
the core concepts from our previous discussion on memory segmentation and
anonymous mapping.

-----

### 1\. Analysis of `pmap -x` Output

The `-x` (extended) option adds three critical columns to the virtual memory
map, showing the link between the **virtual address space** and **physical
RAM** usage:

1. **Kbytes:** The **Virtual Size** of the memory region (how much address
space is reserved).
2. **RSS:** **Resident Set Size**. The actual amount of **physical RAM**
(in KiB) currently committed and held by the process for this segment. This
is the memory that is *not* swapped out.
3. **Dirty:** The amount of **writable** memory (in KiB) in this segment
that has been **modified** by the process since it was loaded or since its
last write-back to disk.
4. **Mode:** The access permissions (`r`, `w`, `x`).
5. **Mapping:** The source of the memory (file-backed or `[ anon ]` anonymous).

-----

### 2\. The Dominance of the Anonymous Heap

The memory allocated by your `memory-user.out` program is immediately
identifiable by its sheer size and its three key metrics:

```bash
00007c2023dff000 1048580 1048576 1048576 rw--- [ anon ]
```

  * **Kbytes (Virtual Size):** $1,048,580 \text{ KiB}$ (exactly $1024 \text{
  MiB}$). This is the size reserved by the `malloc` call.
  * **RSS (Physical RAM):** $1,048,576 \text{ KiB}$. This confirms that
  nearly every single page of the allocated $1024 \text{ MiB}$ virtual space
  is currently backed by **physical RAM**. The slight difference between
  $1,048,580 \text{ K}$ and $1,048,576 \text{ K}$ is likely due to page
  alignment, but it shows near-perfect commitment.
  * **Dirty:** $1,048,576 \text{ KiB}$. This is the most crucial part for
  your study. Because your C program is continuously streaming and writing to
  this memory (thanks to the `volatile` keyword), the kernel marks **all** of
  these pages as **dirty**. This tells the kernel: "This memory is actively
  being used and must be kept in RAM; it is not eligible for reclaiming or
  swapping out."
  * **Mapping:** **`[ anon ]`**. As discussed, this is the process's **Heap**
  area, dynamically allocated and not tied to any disk file.

-----

### 3\. Shared Library Segmentation and Commitment

The `pmap -x` output vividly illustrates the efficiency of shared libraries
by showing the difference between their total size and the memory that must
be unique to your process.

#### A. Shareable Read-Only Segments

Observe the executable code (`r-x`) and read-only data (`r--`) segments of
`libc.so.6` and `ld-linux`:

```bash
00007c2063e28000   1568K   980   0 r-x-- libc.so.6   <-- Code (r-x)
00007c2063fb0000    316K   100   0 r---- libc.so.6   <-- Read-only Data (r--)
00007c206408c000    172K   172   0 r-x-- ld-linux-x86-64.so.2 <-- Linker Code
```

  * **Dirty is Zero (`0`):** The code and read-only data are **never modified**
  by your running process, so the `Dirty` column is `0`.
  * **Shared RAM (Low RSS):** Because they are clean and read-only, the
  physical memory pages backing these segments can be **shared** between
  every process running on your system. This is a massive RAM saving. (Note
  the `libc.so.6` code section only has $980 \text{ KiB}$ of RSS, meaning
  only those parts have been touched and loaded into physical RAM).

#### B. Private Writable Segments

Now look at the writable parts of the libraries:

```bash
00007c2064003000      8K     8     8 rw--- libc.so.6
00007c20640c3000      8K     8     8 rw--- ld-linux-x86-64.so.2
```

  * **RSS Equals Kbytes:** The size of the private writable section (e.g.,
  $8 \text{ K}$) is equal to its RSS ($8 \text{ K}$).
  * **Dirty Equals RSS:** The Dirty value also equals the RSS.

This pattern indicates that these segments are the **private writable copies**
of the library's global variables and internal state. They had to be copied
upon write (Copy-on-Write, or COW) to give your process its own independent,
modifiable state. They are entirely unique to process $494329$ and thus
count fully against its memory usage.

### 4\. The Grand Total

The summary confirms the results:

```bash
total kB           1051264 1050144 1048676
```

  * **Virtual Size (Kbytes):** $1,051,264 \text{ KiB}$ (The total address
  space reserved).
  * **Total RSS:** $1,050,144 \text{ KiB}$. This is the total **physical
  RAM** currently tied up by your process. Again, this is dominated by the
  $1,048,576 \text{ KiB}$ from your anonymous array.
  * **Total Dirty:** $1,048,676 \text{ KiB}$. The majority of this is your
  array, with the small remainder being the writable portions of the stack
  and libraries.

**In conclusion, your `pmap -x` output confirms that your $1024 \text{ MiB}$
array is correctly forcing the Linux kernel to commit the physical RAM, and
your `volatile` access is keeping those pages dirty and resident in memory.**

## Swap memory

That's an excellent summary of your observations, and your confusion about
**swap memory** is completely justified—it's one of the trickiest concepts
in operating systems.

You've captured the roles of **Kbytes**, **RSS**, and **Dirty** almost
perfectly. I'll refine your assertions and then tackle the concept of swapping
and why it's necessary.

---

### Refining Your Memory Assertions

Here is a quick correction and refinement of your assertions about the
`pmap -x` columns:

1. **Kbytes:**
  - **Refined assessment:** The **Virtual Size** of the memory region,
  defined during linking or at runtime (via `malloc`/`mmap`).
  - **Status:** **Correct.** It is the size of the *reservation* in the
  process's address space.
2. **RSS:**
  - **Refined assessment:** The actual **physical RAM commitment**. It can
  be delayed (lazy), and pages can be swapped away.
  - **Status:** **Mostly Correct.** It is the current *physical* commitment. It
  is **not guaranteed** because the kernel can steal or swap the pages to
  keep available memory high.
3. **Dirty:**
  - **Refined assessment:** Physical memory **continuously accessed**.
  - **Status:** **Correction.** The Dirty flag just means the page has been
  **written to** since it was loaded or last backed up. It doesn't necessarily
  mean it's *continuously* accessed, but rather that its contents differ
  from the original file/zero-state and **must be saved** before being reused.

The key takeaway is that the kernel is always prioritizing **RSS** (physical
commitment) over **Kbytes** (virtual reservation) to ensure efficiency.

---

### Understanding Swap Memory

**Swap memory** (or **swap space**) is simply a designated area on a storage
device (usually a hard drive or SSD) that the operating system uses as a
temporary holding area for less-used physical RAM pages.

The entire concept of swapping is the kernel's way of implementing **virtual
memory** management and ensuring that the **Available** memory stays high.

#### 1. Why Swapping is Necessary

The core reason for swap is to handle **oversubscription** and **reclamation**:

* **Oversubscription (More Programs than RAM):** If you try to run programs
that collectively require $12 \text{ GiB}$ of active memory, but your system
only has $8 \text{ GiB}$ of physical RAM, the kernel can use swap to move
$4 \text{ GiB}$ of the least-recently-used pages to disk. This allows the
programs to run, albeit slowly when accessing the swapped-out data.
* **Reclamation for Efficiency:** Linux often swaps out perfectly good, but
old, unused pages from long-running applications (even when there is free RAM)
to free up space for the **page cache**. The page cache (the `buff/cache`
in your `free` output) is crucial for disk I/O performance. Swapping ensures
the kernel has enough free physical RAM to boost file access speed.

#### 2. The Relationship Between Dirty, RSS, and Swap

The kernel's decision to swap a memory page depends on two primary factors:

* **Clean (Not Dirty)**
  - **Kernel Action:** Can be simply **discarded**.
  - **Why:** If a page (like a page of code from `libc.so.6`) hasn't been
  modified, the kernel can simply discard it from RAM. If needed later,
  it can quickly be reloaded from the original file on disk. |
* **Dirty (Modified)**
  - **Kernel Action:** **Must be saved to disk.**
  - **Why:** If a page has been modified (like your array data or stack
  data), the kernel **must** write the modified content to the **swap space**
  before reclaiming the physical RAM page. This is slower but necessary to
  avoid losing data. |

#### 3. The Role of Your Program

Your `memory-user.c` program actively fights the kernel's swapping mechanism:

1.  It forces $1 \text{ GiB}$ into physical RAM (**High RSS**).
2.  It keeps all those pages **Dirty** by constantly writing to them
(`volatile`).
3.  By keeping the pages *recently and continuously* accessed, the memory
management unit flags them as **hot**, making them extremely unlikely to be
chosen as candidates for being swapped out to the slow disk.

Your experiment demonstrates that by keeping memory hot and dirty, you
successfully overcome the OS's normal "lazy" behavior and force the highest
possible **RSS** value.

## What does Kbytes > RSS mean

That is the single most important metric for understanding the efficiency
of a Linux process!

When **RSS is less than Kbytes**, it means your program has reserved a
large amount of virtual address space, but the operating system has not
(yet) committed physical RAM to back all of that space.

This difference is the definition of the Linux kernel's **"lazy" memory
management** in action.

Here is a breakdown of what that discrepancy means, focusing on why the
kernel does this:

---

## 1. The Core Meaning: Virtual vs. Physical

| Metric | Represents | Source |
| :--- | :--- | :--- |
| **Kbytes (Virtual Size)** | The program's **reservation** for memory. | Defined by the linker, or by calls like `malloc()`/`mmap()`. |
| **RSS (Resident Set Size)** | The program's **physical usage** right now. | The actual RAM pages committed by the kernel. |

**If Kbytes > RSS, the OS is holding the unbacked virtual space ready,
but it has not spent the valuable physical RAM yet.**

---

## 2. The Primary Reasons for the Discrepancy

There are three main scenarios where a process reserves a lot of memory but
has low physical commitment:

### A. Lazy Allocation and Paging (The `malloc` Effect)

As we discussed, `malloc()` is lazy. When you allocate $1 \text{ GiB}$, the
kernel marks $1 \text{ GiB}$ in the **Kbytes** column, but the **RSS** will
remain low until your program actually touches (reads or writes) those pages.

* **Example:** A program asks for $4 \text{ GiB}$ of memory for a sparse
matrix.
    * **Kbytes:** $4,194,304 \text{ KiB}$
    * **RSS:** $100 \text{ KiB}$ (Only the initial parts of the matrix and
    program overhead).
* **Rationale:** Why commit $4 \text{ GiB}$ of RAM if the program is only
going to use a fraction of it? The kernel saves that RAM for disk caching or
other active processes until the very moment a page fault proves the memory
is needed.

### B. Shared, Unmodified Code (Code and Libraries)

Most of the code and read-only data from shared libraries are prime examples
of this phenomenon.

* **Scenario:** A large shared library, say $50 \text{ MiB}$ of code, is
linked into your program.
    * **Kbytes:** $51,200 \text{ KiB}$ (This is the size of the whole library).
    * **RSS:** $5,000 \text{ KiB}$ (Only the functions your program has
    actually called so far).
* **Rationale:** The entire library's code is mapped into your virtual
address space (high **Kbytes**), but only the pages containing the functions
you have executed are loaded into physical RAM (**RSS**). The rest of the
library remains on disk until needed.

### C. Swapped-Out Pages

This is the scenario where the pages *were* in physical RAM, but were later
moved to the swap space on the disk.

* **Scenario:** A program allocates $500 \text{ MiB}$ and uses it actively,
so $\text{RSS}$ starts high. The program then sits idle for an hour, and
memory pressure increases on the system.
    * The kernel moves $400 \text{ MiB}$ of the program's data to swap disk.
    * **Kbytes** remains $512,000 \text{ KiB}$ (The reservation never changes).
    * **RSS** drops to $100 \text{ MiB}$.
* **Rationale:** The kernel took the idle physical RAM pages and repurposed
them, but the process still maintains its virtual reservation. If the process
suddenly needs that data back, the kernel will retrieve it from swap (a slow
process called **page faulting**), but it saves the physical RAM for more
important, active tasks in the meantime.

In summary, a high **Kbytes** value simply indicates a potential memory
requirement, while a high **RSS** value indicates the true, costly physical
demand on the system.

