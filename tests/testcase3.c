#include <stdlib.h>

int main() {
    int n = 100000;
    char *allocations[n];

    // Allocate million count, 1 byte piece
    for (int i = 0; i < n; i++) {
        allocations[i] = malloc(1);
    }

    // Free in reverse order
    for (int i = n - 1; i >= 0; i--) {
        free(allocations[i]);
    }

    return 0;
}
