#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int n = 500;
    char *allocations[n];

    // Allocate chunks of varying sizes
    for (int i = 0; i < n; i++) {
        allocations[i] = malloc((rand() % 100) + 1);
    }

    // Free chunks in random order
    for (int i = 0; i < n; i++) {
        int idx = rand() % n;

        if (allocations[idx] != NULL) {
            free(allocations[idx]);
            allocations[idx] = NULL;
        }
    }

    return 0;
}
