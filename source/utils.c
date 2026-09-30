#include "../include/utils.h"


int compare_uint8(const void *a, const void *b) {
    uint8_t ua = *(const uint8_t*)a;
    uint8_t ub = *(const uint8_t*)b;
    return (ua > ub) - (ua < ub);
}

uint8_t find_median_uint8(uint8_t arr[], int n) {
    // 1. Sort the array in ascending order
    qsort(arr, n, sizeof(uint8_t), compare_uint8);
    
    // 2. If odd, return the middle element
    if (n % 2 != 0) {
        return arr[n / 2];
    } 
    // 3. If even, return the rounded average of the two middle elements
    else {
        int sum = arr[(n / 2) - 1] + arr[n / 2];
        return (uint8_t)((sum + 1) / 2); // (sum + 1) / 2 handles correct integer rounding
    }
}