# Fixing `rebuild_all_macos.sh` on Apple Silicon

`rebuild_all_macos.sh` and `rebuild_all.sh` are functionally identical (the
macOS variant only hardcodes `CPU_COUNT` instead of reading `/proc/cpuinfo`).
The failure was never in these wrapper scripts — it was five separate issues
in the bundled host toolchain (binutils 2.32 / GCC 6.5.0 / GMP 6.1.2, all
vintage 2016–2019) that only surface when building on macOS, and specifically
on Apple Silicon. Diagnosed and fixed by actually running the build on an
M-series Mac (macOS 26, arm64) and iterating on each failure in turn.

## 1. Ancient bundled zlib collides with the macOS SDK's `fdopen`

**Symptom:** `error: expected identifier or '('` deep in `zlib/zutil.c`
while building host binutils and GCC.

**Cause:** binutils and GCC each vendor a fallback copy of zlib 1.1.4.
Its `zutil.h` has:
```c
#if defined(MACOS) || defined(TARGET_OS_MAC)
#      define fdopen(fd,mode) NULL /* No fdopen() */
#endif
```
`TARGET_OS_MAC` is an Apple SDK macro that's defined on *every* Apple
platform, including modern macOS — but this code predates Mac OS X and meant
classic (pre-OS X) Mac OS, where `fdopen()` genuinely didn't exist. On real
macOS it fires anyway, `#define`-ing `fdopen` to `NULL`, which then mangles
the real `fdopen()` prototype pulled in from `<stdio.h>`.

**Fix:** guard the branch with `&& !defined(__APPLE__)` in both vendored
copies (`tools/crosstools/gnu/binutils-2.32-aros.diff`,
`tools/crosstools/gnu/gcc-6.5.0-aros.diff`).

## 2. Vendored `config.guess` predates Apple Silicon

**Symptom:** `configure: error: Oops, mp_limb_t is 64 bits, but the
assembler code in mpn/ won't work on this system-abi combination` while
configuring GMP.

**Cause:** binutils/GCC/GMP each carry their own copy of GNU's
`config.guess`/`config.sub`, all from before Apple Silicon existed (Nov
2020). On an M-series Mac, `uname -m` reports `arm64`, which these old
scripts don't recognize — they fall back to a bare `arm` (implying 32-bit),
producing the triple `arm-apple-darwinNN`. GMP then detects a real 64-bit
`mp_limb_t` at compile time, sees a 32-bit-implying host triple, and aborts.

**Fix:** stop relying on the vendored `config.guess` entirely for Darwin.
`tools/crosstools/gnu/mmakefile.src` now passes an explicit, correct
`--build=$(AROS_HOST_CPU)-apple-darwin --host=$(AROS_HOST_CPU)-apple-darwin`
to every host-compiler sub-package (gmp, isl, mpfr, mpc, binutils, gcc) when
`$(AROS_HOST_ARCH)` is `darwin`. This bypasses the broken auto-detection.

## 3. GMP 6.1.2 has no Apple Silicon assembly

**Symptom:** `error: unknown AArch64 fixup kind!` / `ADR/ADRP relocations
must be GOT relative` compiling `invert_limb.lo`, once (2) let GMP correctly
detect an aarch64 host.

**Cause:** GMP 6.1.2's hand-written AArch64 assembly assumes a Linux/ELF
assembler. It was written years before Apple Silicon existed and doesn't
know about Apple's Mach-O/PIE relocation model, which Apple's assembler
enforces.

**Fix:** pass `--disable-assembly` to GMP on Darwin (same mmakefile.src
conditional as above), forcing its portable C implementation. Only affects
build speed of the host compiler itself, not the resulting m68k target code.

## 4. GCC's own `fibonacci_heap.h` has a latent bug

**Symptom:** `error: reference to non-static member function must be
called` in `gcc/fibonacci_heap.h:481` while compiling `bb-reorder.o`.

**Cause:** genuine bug in upstream GCC 6.5.0. `fibonacci_heap<K,V>` has a
member function `V *min()`, but `union_of()` uses it as if it were a data
member:
```cpp
if (heapb->min->compare (heapa->min) < 0)   // should be heapb->min()->compare(heapa->min())
```
GCC's own (lenient) C++ frontend tolerates this; Apple Clang — a much
stricter, standards-conformant compiler — correctly rejects it. This is why
Linux (host-compiled by GCC) builds fine and macOS (host-compiled by Clang)
doesn't.

**Fix:** call `min()` properly:
`heapb->min ()->compare (heapa->min ()) < 0` (patched into
`gcc-6.5.0-aros.diff`).

## 5. GCC has no host-hooks object for Apple Silicon

**Symptom:** `Undefined symbols for architecture arm64: "_host_hooks"`
linking `cc1`.

**Cause:** GCC's generic `gcc/config/host-darwin.c` only supplies PCH helper
functions — the actual `const struct host_hooks host_hooks = ...;`
definition is expected to come from a CPU-specific companion file
(`config/i386/host-i386-darwin.c` for Intel, an rs6000 equivalent for
PowerPC). `gcc/config.host` never gained an `aarch64-*-darwin*` /
`arm64-*-darwin*` case, because that hardware didn't exist yet, so no
companion object is built and `host_hooks` is left undefined.

**Fix:** added `gcc/config/aarch64/host-aarch64-darwin.c` and
`gcc/config/aarch64/x-darwin` (mirroring the i386 versions exactly), and a
matching case in `gcc/config.host`.

## 6. Apple Silicon's 16KB page size breaks GCC's PCH buffer

**Symptom:** `internal compiler error: in darwin_gt_pch_use_address, at
config/host-darwin.c:51` — every invocation of the freshly-built `xgcc`
crashes, including the trivial "can this compiler compile anything"
autoconf probe for target libgcc.

**Cause:** `host-darwin.c` reserves a static 1GB buffer for precompiled
headers, aligned to a hardcoded 4096 bytes:
```c
static char pch_address_space[1024*1024*1024] __attribute__((aligned (4096)));
```
then asserts the buffer's address is a multiple of the *runtime* page size.
Every Mac this code was ever tested on used 4KB pages — but Apple Silicon
uses 16KB pages. 4096-byte alignment doesn't guarantee 16384-byte alignment,
so the assertion can (and did) fail.

**Fix:** bump the alignment to 16384 in `gcc-6.5.0-aros.diff`. 16384 is also
a multiple of 4096, so this is safe on Intel Macs too.

## 7. `mkisofs` isn't installed

**Symptom:** `/bin/sh: mkisofs: command not found` building
`distfiles/aros-amiga-m68k.iso`.

**Cause:** not a code bug — `dependencies_macos.sh` already lists
`cdrtools` (which provides `mkisofs`), it just hadn't been installed on this
machine.

**Fix:** `brew install cdrtools`.

## Result

With all of the above, `rebuild_all_macos.sh`'s underlying build steps run
to completion on Apple Silicon: the host GCC 6.50 cross-compiler builds, the
full AROS/ApolloOS tree builds (`make -j`), `make distfiles` produces
`distfiles/aros-amiga-m68k.iso`, and `ApolloROM.V4` assembles correctly.

Files changed:
- `tools/crosstools/gnu/binutils-2.32-aros.diff` — zlib `fdopen` guard fix.
- `tools/crosstools/gnu/gcc-6.5.0-aros.diff` — zlib guard, `fibonacci_heap.h`
  fix, aarch64 host-hooks files + `config.host` case, PCH alignment fix.
- `tools/crosstools/gnu/mmakefile.src` — explicit `--build`/`--host` triple
  and `--disable-assembly` for Darwin host-compiler sub-builds.

Not committed (local machine setup, not a repo bug):
- `brew install cdrtools` for `mkisofs`.
