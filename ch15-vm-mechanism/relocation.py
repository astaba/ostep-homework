#!/usr/bin/env python

from __future__ import print_function

import random
from optparse import OptionParser


# to make Python2 and Python3 act the same -- how dumb
def random_seed(seed):
    """
    Seed the global random number generator with a stable, forward-compatible value.

    This helper calls random.seed(seed, version=1) when the runtime's random
    implementation accepts the 'version' keyword argument; if that call raises
    an exception (for example on older Python builds that do not accept the
    'version' parameter), it falls back to calling random.seed(seed).

    Parameters
    ----------
    seed : int | float | str | bytes | bytearray | None
        The seed value to initialize the random number generator. See the
        `random.seed` documentation for allowed types and behavior. Passing
        None will seed from system entropy.

    Returns
    -------
    None
        This function does not return a value; it configures the module-level
        state of the Python `random` module.

    Notes
    -----
    - The function intentionally swallows errors from attempting the versioned
      call in order to provide compatibility across Python/runtime versions.
    - Any validation or errors raised by the underlying `random.seed` call
      (for unsupported seed types) will propagate from that call.

    Examples
    --------
    >>> random_seed(12345)
    >>> random.random()  # subsequent calls are deterministic given the seed
    """
    try:
        # Prefer the versioned seed call when available (Python 3.2+).
        random.seed(seed, version=1)
    except:
        # Fall back to the simple call for older runtimes.
        random.seed(seed)
    return

def convert(size):
    """
    Convert a human-readable size string to an integer number of bytes.

    Accepts values like "16", "4k", "32m", "1g". Interprets K/M/G as binary
    multiples (1024, 1024**2, 1024**3 respectively). Returns integer bytes.

    Note: this helper is intentionally simple: it does not accept fractions
    (e.g. "1.5K") and expects a non-empty string.
    """
    # normalize and inspect last character to determine units
    lastchar = size[-1].lower()
    if lastchar == 'k':
        # kilobytes (kibibytes) -> multiply by 1024
        m = 1024
        nsize = int(size[:-1]) * m
    elif lastchar == 'm':
        # megabytes (mebibytes) -> multiply by 1024*1024
        m = 1024*1024
        nsize = int(size[0:-1]) * m
    elif lastchar == 'g':
        # gigabytes (gibibytes) -> multiply by 1024**3
        m = 1024*1024*1024
        nsize = int(size[0:-1]) * m
    else:
        # no suffix -> treat as plain integer number of bytes
        nsize = int(size)
    return nsize


#
# main program
#
parser = OptionParser()

# Option definitions:
# -s / --seed: deterministic seed for the pseudo-random generator. Useful to reproduce the same sequence of addresses for debugging or tests.
parser.add_option('-s', '--seed', default=0, help='the random seed', action='store', type='int', dest='seed')
# -a / --asize: virtual address space size for the simulated process. Accepts human readable suffixes handled by convert().
parser.add_option('-a', '--asize', default='1k', help='address space size (e.g., 16, 64k, 32m, 1g)', action='store', type='string', dest='asize')
# -p / --physmem: size of physical memory for the simulation. Must be larger than the address space for this simple exercise.
parser.add_option('-p', '--physmem', default='16k', help='physical memory size (e.g., 16, 64k, 32m, 1g)', action='store', type='string', dest='psize')
# -n / --addresses: how many virtual addresses to generate in the trace.
parser.add_option('-n', '--addresses', default=5, help='number of virtual addresses to generate', action='store', type='int', dest='num')
# -b / --b: optional explicit base register value (as string). Default '-1' indicates "generate randomly".
parser.add_option('-b', '--b', default='-1', help='value of base register', action='store', type='string', dest='base')
# -l / --l: optional explicit limit register value (as string). Default '-1' indicates "generate randomly".
parser.add_option('-l', '--l', default='-1', help='value of limit register', action='store', type='string', dest='limit')
# -c / --compute: if present, the script will compute and print the physical
#                 address translations or indicate segmentation violations.
parser.add_option('-c', '--compute', default=False, help='compute answers for me', action='store_true', dest='solve')


(options, args) = parser.parse_args()

# Echo the parsed arguments so the user knows what the simulation will use.
print('')
print('ARG seed', options.seed)
print('ARG address space size', options.asize)
print('ARG phys mem size', options.psize)
print('')

# Seed the RNG to make behavior reproducible when a seed is supplied.
random_seed(options.seed)

# Convert human-friendly sizes into integer byte counts.
asize = convert(options.asize)   # virtual address space size (bytes)
psize = convert(options.psize)   # physical memory size (bytes)

# Basic sanity checks for the simulation parameters:
if psize <= 1:
    # physical memory must be non-trivial
    print('Error: must specify a non-zero physical memory size.')
    exit(1)

if asize == 0:
    # virtual address space must be non-zero for the exercise
    print('Error: must specify a non-zero address-space size.')
    exit(1)

if psize <= asize:
    # For this toy simulation we require physical memory strictly greater
    # than the address space to make random placement of the segment easier.
    print('Error: physical memory size must be GREATER than address space size (for this simulation)')
    exit(1)

#
# need to generate base and limit for the segment (base-and-bounds)
#
# The options were parsed as strings to allow '-1' sentinel; convert() will
# turn them into integer sizes. A user can also pass an explicit numeric
# value for base/limit.
limit = convert(options.limit)
base  = convert(options.base)

# If limit was left as the sentinel (-1), generate a reasonable default:
# choose a size between 1/4 and 1/2 of the address space:
#   limit = floor(asize/4 + (asize/4 * random_float))
# where random_float is in [0.0, 1.0)
# Resulting range: [asize/4, asize/2)
if limit == -1:
    limit = int(asize/4.0 + (asize/4.0 * random.random()))

# If base was left as the sentinel (-1), pick a random base in physical
# memory such that the region [base, base+limit) fits entirely in psize.
if base == -1:
    done = 0
    while done == 0:
        # pick random base in [0, psize)
        base = int(psize * random.random())
        # accept it only if the segment fits in physical memory
        if (base + limit) < psize:
            done = 1

# Print the chosen base-and-bounds registers for the simulated process.
print('Base-and-Bounds register information:')
print('')
print('  Base   : 0x%08x (decimal %d)' % (base, base))
print('  Limit  : %d' % (limit))
print('')

# Final sanity check: ensure the segment fits in physical memory.
if base + limit > psize:
    print('Error: address space does not fit into physical memory with those base/bounds values.')
    print('Base + Limit:', base + limit, '  Psize:', psize)
    exit(1)

#
# now, generate a virtual address trace and optionally compute translations
#
print('Virtual Address Trace')
for i in range(0, options.num):
    # produce a random virtual address in the virtual address space:
    # random.random() returns a float in [0.0, 1.0); multiplying by asize
    # and truncating with int() yields an integer in [0, asize-1].
    vaddr = int(asize * random.random())

    if options.solve == False:
        # When not asking for answers, print the VA and ask the student to
        # determine whether it's a segmentation violation or a valid translation.
        print('  VA %2d: 0x%08x (decimal: %4d) --> PA or segmentation violation?' % (i, vaddr, vaddr))
    else:
        # Compute whether the VA is in bounds, and if so compute the PA.
        paddr = 0
        if (vaddr >= limit):
            # If VA >= limit the address is outside the process segment.
            print('  VA %2d: 0x%08x (decimal: %4d) --> SEGMENTATION VIOLATION' % (i, vaddr, vaddr))
        else:
            # Valid VA: physical address is base + vaddr
            paddr = vaddr + base
            print('  VA %2d: 0x%08x (decimal: %4d) --> VALID: 0x%08x (decimal: %4d)' % (i, vaddr, vaddr, paddr, paddr))

print('')

# Additional explanatory text printed when not computing answers.
if options.solve == False:
    print('For each virtual address, either write down the physical address it translates to')
    print('OR write down that it is an out-of-bounds address (a segmentation violation). For')
    print('this problem, you should assume a simple virtual address space of a given size.')
    print('')
