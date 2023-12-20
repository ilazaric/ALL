# alloc

suppose we have an allocator implementation
deallocation does work actually
consider N allocations of size 1
then consider a subset of deallocations, for each subset
2^N different situations
group situations by size of subset
k -> (N choose k) subsets
contributes (N choose k) / 2^(CHAR\_BIT * k)
sum[k=0..N] { (N choose k) * 2^(-CHAR\_BIT) ^k } =
(1 + 2^(-CHAR_BIT))^N
--> linear extra memory needed
