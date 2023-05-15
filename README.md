


# Custom Memory Allocator


This is a custom memory allocator implemented in C. It uses a simple, efficient data structure called a free list to keep track of free blocks of memory that can be allocated. 

The memory allocator offers the following features:

- First Fit, Best Fit, Worst Fit, and Next Fit allocation strategies.
- Coalescing free blocks of memory to reduce fragmentation.
- Splitting blocks of memory to closely fit requested sizes.
- The ability to grow the heap if needed.

The allocator provides the following API:

- `void* malloc(size_t size)`: Allocates a block of memory of the given size.
- `void free(void *ptr)`: Frees the memory block pointed to by `ptr`.
- `void* calloc(size_t nmemb, size_t size)`: Allocates an array of `nmemb` elements, each of the size `size`.
- `void* realloc(void *ptr, size_t size)`: Changes the size of the memory block pointed to by `ptr` to `size` bytes.

The code will print the statistics upon exit and should look like similar to:
```
mallocs: 8
frees: 8
reuses: 1
grows: 5
splits: 1
coalesces: 1
blocks: 5
requested: 7298
max heap: 4096
```


## Building and Running the Code

The code compiles into four shared libraries and six test programs. To build the code, change to your top level assignment directory and type:
```
make
```
Once you have the library, you can use it to override the existing malloc by using
LD_PRELOAD. The following example shows running the ffnf test using the First Fit shared library:
```
$ env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf
```

To run the other heap management schemes replace libmalloc-ff.so with the appropriate library:
```
Best-Fit: libmalloc-bf.so
First-Fit: libmalloc-ff.so
Next-Fit: libmalloc-nf.so
Worst-Fit: libmalloc-wf.so
```

To run the tests, make sure to compile and run your test suite. There are four test cases under the tests directory.

## Test Cases

The allocator's performance and correctness are tested using the following test cases:

- `TestCase1 (CoalesceTesting)`: This test allocates 500 chunks of 100 bytes each, assigns random values, and then frees every other chunk. Then, it frees the remaining chunks. This tests the allocator's ability to merge adjacent free blocks and reduce wasted memory.

- `TestCase2 (ReuseTesting)`: This test allocates 0.5 GB of memory at once, then frees it and allocates smaller parts. It tests whether the allocator can efficiently reuse the big freed block for smaller requests.

- `TestCase3 (100KSmallAllocations)`: This test allocates a million tiny 1-byte memory blocks. It tests the allocator's performance when handling lots of small allocations, which can lead to wasted memory and slower allocation times.

- `TestCase4 (GrowingAllocations)`: This test allocates memory blocks with increasing sizes, then frees them and reallocates them with decreasing sizes. It tests the allocator's ability to handle different allocation sizes and efficiently reuse freed memory.



## Performance Comparison

Here is a comparison of the time taken by each allocation strategy and the default system allocator for each test case:

|           | Test Case 1 | Test Case 2 | Test Case 3 | Test Case 4 |
|-----------|-------------|-------------|-------------|-------------|
| First Fit | 0.011s      | 2.019s      | 42.998s     | 0.006s      |
| Best Fit  | 0.007s      | 2.077s      | 42.835s     | 0.006s      |
| Worst Fit | 0.009s      | 2.088s      | 43.145s     | 0.006s      |
| Next Fit  | 0.016s      | 0.028s      | 42.104s     | 0.004s      |
| Default   | 0.004s      | 0.134s      | 0.008s      | 0.005s      |

This table gives you a quick overview of the performance of each strategy. As you can see, the time taken varies depending on the allocation strategy and the type of allocation requests being made.

## Copyright 

Copyright © 2023. All rights reserved.

This software is provided for educational purposes only. It is prohibited to use this code, for any college assignment or personal use. Unauthorized distribution, modification or commercial usage is strictly forbidden. Please respect the rights of the author.


