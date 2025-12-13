stl allocator concept sucks  
we introduce different stuff  

# single object allocators  

a type `T` is a single object allocator for type `U` if  
informally we can generate `U`-sized&aligned memory (and nothing else)  
with it

we dont have to always be able to allocate, it can fail  

how to fail?  
exception?  
resulty return?  
hmmmmmmm  
resulty -> exception emulation? easy  
exception -> resulty emulation? harder? wrap into a std::any?  
lets go with exception, but also provide queries  
would be nice to just return a nullptr hmmmmm  
but a nullptr might mess with fancy ptrs, a no-addr must be representible  
wait, why not all APIs?  
hmm, but this would require ptrs to be default-constructible?  
hmmmmmmmm  

API for regular, non-fancy:  
```cpp
T alloc;
std::same_as<T::value_type, U>;
alloc.can_allocate() -> bool;
alloc.allocate() -> U*; // can throw, can't throw if can_allocate() said true right before
alloc.deallocate(U*) -> void; // prereq: arg came from allocate(), wasn't deallocate()'d before
```

should it be able to say more?  
like "how many minimum can this allocate?" ??  
maybe neither, both should be "extension functions", optional shit  

API for generic interface:  
```cpp
// calls alloc.can_allocate() if possible, false otherwise
// if allocator provides this, and can_allocate() returns true,
// a subsequent call to allocate() must never throw
// notably, a false does not mean "definitely throws"
allocator_interface::can_allocate(T&) -> bool;

// calls alloc.can_allocate_number() if possible, 0 otherwise
// getting N from this, it must be true that calling allocate()
// N times in a row must never throw
// TODO: probably elaborate somehow that deallocate() doesn't break this chain
// TODO: should this fallback onto can_allocate() to get a bit of info?
allocator_interface::can_allocate_number(T&) -> size_t;

allocator_interface::allocate(T&) -> U*; // this one being optional sounds weird, but kinda works i guess, just throw
allocator_interface::deallocate(T&, U*) -> void; // even this could be optional haha
```

should allocator_interface be a namespace?  
a class?  
lets go with class with static fns, but unsure this is better


what about fancy ptrs?  
probably want 2 kinds  
nullable and non-nullable  
nullable subset of non-nullable  
nullable iff default constructible  
wait  
need to remember `T t{};` is not same as `T t;`  

fancy pointer API:  
```cpp
Ptr::value_type;
*ptr -> Ptr::value_type;
operator->() -> Ptr::value_type*;
copyable
equality-comparable
```

nullable ptr API:  
```cpp
value-initializable;
pointer API (* and -> prereq: non-default);
```

should i just require nullability always?  
lets go with yes


