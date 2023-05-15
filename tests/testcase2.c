#include <stdlib.h>

int main() {
    // Allocate 0.5 gig in one go and free it
    char *large_allocation = malloc(500 * 1024 * 1024);
    free(large_allocation);

    // Allocate smaller partitions
    int n = 10000;
    char *small_allocations[n];

    for (int i = 0; i < n; i++) {
        small_allocations[i] = malloc(50 * 1024);
    }

    // Free smaller partitions
    for (int i = 0; i < n; i++) {
        free(small_allocations[i]);
    }

    return 0;
}
