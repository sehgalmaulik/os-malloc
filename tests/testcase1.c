#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int n = 500;
    char *allocations[n];

    // Allocate 500 count, 100 byte chunks, and assign random values
    for (int i = 0; i < n; i++) {
        allocations[i] = malloc(100);
        for (int j = 0; j < 100; j++) {
            allocations[i][j] = rand() % 256;
        }
    }

    // Free alternate allocations
    for (int i = 0; i < n; i += 2) {
        free(allocations[i]);
    }

    // Free the remaining allocations
    for (int i = 1; i < n; i += 2) {
        free(allocations[i]);
    }

    return 0;
}
