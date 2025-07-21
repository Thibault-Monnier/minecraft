#pragma once
#include <cstdint>

// inline uint64_t perfNbCycles = 0;
// inline uint64_t perfNbIterations = 0;

inline uint64_t get_cycles() {
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<uint64_t>(hi) << 32) | lo;
}
