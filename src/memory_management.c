// POSSIBILITIES:
// - Have physical pages used by kernel always mapped.
//   - This would mean that these virtual addresses couldn't be used elsewhere...
//   - But, it does mean that we don't have to worry about on-demand page allocation
//     and could make memory allocation in the kernel really quick.
//
// - If we do on-demand paging:
//   - We need to know how much space we have on the last allocated page.
//   - If size is more than the space left, we allocate a number of pages to hold size.
//   - We will need to map them adjacent in virtual memory, of course.

void *kernel_allocate(size_t size) {
    
}
