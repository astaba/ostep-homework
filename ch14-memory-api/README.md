# Memory API

## Vector-like structure

> **Definition:** In **C** once you declare `arr[10]` at compile time
it's no longer possible for `arr` to hold 11 elements unless you declare
a new array. That is where **vector** shows up: **a contiguous sequence of
data elements that can grow and shrink at runtime.** Then a **vector-like
structure** is just a structure that wraps a raw array but manage memory
dynamically using `malloc()` and `realloc()`.

> **Homework 14.8 Question:** How well does such a vector perform? How does
it compare to a linked list?

This comparison relies on a concept called **algorithmic efficiency**, which
is measured using **Big O Notation** (like $O(1)$ or $O(n)$).

---

## 1. Understanding Performance: Big O Notation

Algorithmic efficiency (or **Big O Notation**) is just a way to describe how
the time or memory required for an operation changes as the amount of data
(which we call **N**) grows larger.

Think of **N** as the number of items in your data structure.

### **$O(1)$ - Constant Time (The Fastest)**

* **Meaning:** The operation takes the **same amount of time**, no matter
if N is 10 or 10 billion. The time is *constant*.
* **Analogy:** You need to check the **first name** on a huge, ordered list
of student names. It takes the same time whether the list has 10 students
or 10,000. You just look at the top.

### **$O(n)$ - Linear Time (Scales with Size)**

* **Meaning:** The time taken grows **directly and proportionally** with
the number of items, N. If N doubles, the time taken roughly doubles.
* **Analogy:** You need to find a specific student's name on that same huge
list, but the list is **not alphabetized**. You have to look at **every
single name** from top to bottom. If the list is twice as long, it takes
you twice as long to search.

---

## 2. Vector Performance (The `realloc` Array)

Your vector-like structure stores its elements right next to each other in
**contiguous memory**. This is the source of both its greatest strength and
its major weakness.

| Operation | Performance | Explanation |
| :--- | :--- | :--- |
| **Access/Read (e.g., `vec[5]`)** | **$O(1)$ (Constant)** | This is the vector's super-power. Because all elements are tightly packed, the computer can instantly calculate the exact memory address of the 5th element and jump straight to it. |
| **Insert at End (Normal)** | **$O(1)$ (Constant)** | If the vector has space left, you just write the new data into the next empty slot and increment the `size`. Fast and simple. |
| **Insert at End (Resizing)** | **$O(n)$ (Linear)** | This is the slow part. When the `capacity` is exceeded, you must call `realloc()`. The system might need to copy **all N elements** to a completely new location in memory. |
| **Insert/Delete in Middle** | **$O(n)$ (Linear)** | To add an element at index 3, you must physically shift elements 4, 5, 6, and so on, one spot over to make room. This shifting takes time proportional to the number of elements you move (N). |

### **The Trick of "Amortized $O(1)$"**

Although resizing costs $O(n)$, this cost is spread out over many, many
insertions. When you use the strategy of **doubling the capacity** with
`realloc`, you only incur the expensive $O(n)$ copy operation rarely.

Because of this doubling strategy, the average cost of adding an element
to the end of a vector is considered **$O(1)$ (Constant Time)**, which is
incredibly efficient.

---

## 3. Linked List Performance

A **Linked List** is structured differently. It stores elements in individual
"nodes" scattered randomly across memory. Each node holds the data and a
**pointer** (address) to the next node in the sequence.

| Operation | Performance | Explanation |
| :--- | :--- | :--- |
| **Access/Read (e.g., `list[5]`)** | **$O(n)$ (Linear)** | This is the linked list's major weakness. You can't jump directly to element 5. You must start at the head, follow the first pointer, then the second, then the third... you must **traverse N nodes** to find the Nth element. |
| **Insert/Delete at Ends (Head)** | **$O(1)$ (Constant)** | If you have a pointer to the start of the list, adding a new node there is instant: just create the new node, point it to the old head, and set the list's head to your new node. You only change a few pointers. |
| **Insert/Delete in Middle** | **$O(n)$ (Linear)** | While the physical insertion/deletion (changing the pointers) is $O(1)$, the total time is dominated by the need to **first search** for the correct location, which takes $O(n)$ time. |

---

## 4. Comparison Summary

The choice between a Vector and a Linked List depends entirely on what you
plan to do most often.

| Operation | Vector (`realloc` Array) | Linked List | Which is Better? |
| :--- | :--- | :--- | :--- |
| **Finding / Reading Data** | **$O(1)$ (Instant)** | $O(n)$ (Slow) | **Vector** |
| **Inserting at the End** | $O(1)$ (Amortized) | $O(1)$ (Instant) | **Tie** |
| **Inserting in the Middle** | $O(n)$ (Slow: requires shifting) | $O(n)$ (Slow: requires searching) | **Vector (usually easier to implement)** |
| **Memory Locality** | **Excellent** (Good for CPU caches) | Poor (Scattered memory) | **Vector** |

### **Conclusion**

* **Use the Vector:** If you know the size won't change often, or if you
will be **accessing/reading data by index** (e.g., `my_list[i]`) much more
often than inserting/deleting in the middle. Vectors are the default choice
for most general-purpose lists because reading is fast.
* **Use the Linked List:** If you need to perform many insertions and
deletions, especially at the *very front* of the list, and you **rarely**
need to access elements by their index.

Your `realloc`-based vector is a high-performance structure, making it the
preferred choice over a linked list in most programming scenarios!
