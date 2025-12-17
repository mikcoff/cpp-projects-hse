[[clang::always_inline]]
inline int FastSearch16(const int* __restrict ptr, int value) {
    // Assume ptr is 64-byte aligned and ptr[65536] is a sentinel (INT_MAX)
    
    // Prefetch strategy: near nodes (t0), far nodes (nta)
    __builtin_prefetch(ptr + 4096, 0, 3);   // t0 - near future
    __builtin_prefetch(ptr + 16384, 0, 0);  // nta - far
    __builtin_prefetch(ptr + 49152, 0, 0);  // nta - very far
    
    const int* base = ptr;
    int offset = 0;
    
    // Parallel computation of top 4 bits (reduces dependency chain)
    int bit15 = (base[32768] <= value) * 32768;
    int bit14 = (base[bit15 + 16384] <= value) * 16384;
    int bit13 = (base[bit15 + bit14 + 8192] <= value) * 8192;
    int bit12 = (base[bit15 + bit14 + bit13 + 4096] <= value) * 4096;
    offset = bit15 + bit14 + bit13 + bit12;
    
    // Prefetch next likely cache lines based on current offset
    __builtin_prefetch(base + offset + 1024, 0, 3);
    __builtin_prefetch(base + offset + 256, 0, 3);
    
    // Continue branchless binary search
    offset += (base[offset + 2048] <= value) * 2048;
    offset += (base[offset + 1024] <= value) * 1024;
    offset += (base[offset + 512] <= value) * 512;
    offset += (base[offset + 256] <= value) * 256;
    
    __builtin_prefetch(base + offset + 64, 0, 3);
    
    offset += (base[offset + 128] <= value) * 128;
    offset += (base[offset + 64] <= value) * 64;
    offset += (base[offset + 32] <= value) * 32;
    offset += (base[offset + 16] <= value) * 16;
    offset += (base[offset + 8] <= value) * 8;
    offset += (base[offset + 4] <= value) * 4;
    offset += (base[offset + 2] <= value) * 2;
    offset += (base[offset + 1] <= value) * 1;
    
    // With sentinel at ptr[65536], no bounds check needed
    return (base[offset] == value) ? offset : -1;
}