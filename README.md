# mmap_allocator
Memory allocator built on top of mmap.

Even though `mprotect` on Linux permits changing the protection of memory on the heap to be executable, it is considered a security risk. Some configurations of SELinux will actively prevent this from happening by failing on the `mprotect`. On platforms where this is the case (such as CentOS and RedHat), rather than using `malloc` and `mprotect`, anonymous executable memory can be allocated using `mmap`. Fundamentally, that is what this allocator does.

This code works, but it's meant as a reference and contains design decisions that may not apply to every use case.
