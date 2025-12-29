# Syscall measurement: Two methods

1. Method 1: [OSTEP advised](./syslaps-v1.c)
2. Method 2: [`RDTSC` based](./syslaps64/syslaps-v2.c)

> **Method 1 subtracts loop overhead explicitly; Method 2 implicitly amortizes
it away because `RDTSC` measures only cycles actually retired on the core.**

Now let’s justify that sentence properly.

---

## 1️⃣ What the two methods are *really* measuring

### Method 1 (gettimeofday + subtraction)

You measure:

```
Total_time = N × (loop + syscall)
Empty_loop_time = N × loop
--------------------------------
Syscall_time = N × syscall
```

This is a **differential measurement**.

Key properties:

* Low-resolution clock (µs)
* High amplification (large N)
* Loop overhead must be explicitly removed
* Sensitive to scheduler noise, preemption, migration

---

### Method 2 (`RDTSC` + tight asm loop)

You measure:

```
Total_cycles = N × (syscall + tiny_loop)
Average_cycles ≈ syscall_cycles
```

This is an **amortized measurement**.

Key properties:

* Sub-cycle precision
* Measures *retired CPU cycles*
* Loop overhead is *tiny and constant*
* No need for subtraction if loop cost ≪ syscall cost

---

## 2️⃣ Why the empty loop “doesn’t matter” in Method 2

Let’s quantify.

### Your loop body:

```asm
dec R8D
jnz .loop_read
```

That’s:

* 1 integer ALU op
* 1 predictable branch

Cost (modern x86):

* ~1 cycle per iteration (often less due to fusion)

### Syscall cost:

* ~150–400 cycles (entry + kernel + exit)

So per iteration:

```
Loop cost     ≈   1–2 cycles
Syscall cost ≈ 200+ cycles
```

Relative error:

```
loop / syscall ≈ 0.5–1%
```

And then you average over **1,000,000 iterations**.

➡️ The loop overhead is:

* constant
* deterministic
* dwarfed by syscall cost
* absorbed into the average

That’s why subtraction is *optional* here.

---

## 3️⃣ Why `RDTSC` makes this possible (and gettimeofday doesn’t)

### gettimeofday():

* Quantized (µs)
* Wall-clock based
* Includes:

  * scheduling delays
  * interrupts
  * migration jitter
* Cannot resolve sub-microsecond differences

So you *must* subtract known overheads.

---

### `RDTSC`:

* Counts **actual cycles executed on the core**
* Sub-nanosecond resolution
* Immune to timer quantization
* Measures:

  * instruction retirement
  * pipeline stalls
  * syscall entry/exit cost

This means:

> If something doesn’t burn cycles, `RDTSC` doesn’t “see” it.

That’s the key philosophical difference.

---

## 4️⃣ Why the results match anyway

Both methods converge because:

### Method 1:

* Large N
* Explicit correction
* Low resolution → brute-force averaging

### Method 2:

* Small loop overhead
* Ultra-high resolution
* Hardware-level measurement

They are solving the **same signal-to-noise problem** using different tools.

Mathematically:

```
lim (N→∞)  (Total_time − Loop_time)/N
=
lim (N→∞)  Total_cycles/N
```

Different paths → same limit.

---

## 5️⃣ Why your `RDTSC` frequency calibration works (but is not perfect)

You did:

```c
cycle0 = rdtsc();
clock_gettime();
sleep(1);
clock_gettime();
cycle1 = rdtsc();
```

This works **because**:

* Modern x86 uses **constant-rate TSC**
* TSC does *not* scale with CPU frequency
* Sleep gives you a long, stable interval

But subtle caveats:

* CPU migration could skew results (minor)
* SMT contention can add noise
* Turbo doesn’t affect TSC, only instructions/sec

For teaching purposes: ✔️ excellent.

---

## 6️⃣ Why Method 2 is *conceptually cleaner*

If we rank them:

| Criterion           | Method 1 | Method 2  |
| ------------------- | -------- | --------- |
| Precision           | ❌ µs     | ✅ cycles  |
| Noise resistance    | ❌        | ✅         |
| Loop correction     | Required | Optional  |
| Kernel visibility   | Partial  | Full      |
| Pedagogical clarity | Good     | Excellent |

Method 1 teaches **measurement hygiene**.
Method 2 teaches **hardware truth**.

---

## 7️⃣ One subtle but important point (advanced)

Your Method 2 actually measures:

> **The cost of a syscall when the instruction cache, TLB, and branch
predictors are already hot.**

Method 1, depending on scheduling, may occasionally:

* cold-start kernel paths
* hit page faults
* include involuntary preemption

That Method 2 is *more stable* is expected.

---

## 8️⃣ When the two would *diverge*

They would disagree if:

* The syscall were extremely cheap (e.g., vDSO)
* The loop body were heavy
* The thread migrated across cores without invariant TSC
* Power management broke TSC invariance (rare today)

---

## 9️⃣ Final takeaway (the mental model)

> **gettimeofday measures time as observed by the OS.
> `RDTSC` measures work as done by the CPU.**

You eliminated loop overhead *explicitly* in Method 1.
You eliminated it *by scale* in Method 2.

That’s why the numbers agree.

---
