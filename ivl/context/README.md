
`std::unique_ptr` has a deleter (basically an allocator)  
a `std::vector` has an allocator  
a `std::string` has an allocator  
so a `std::pmr::vector<std::pmr::string>` is really wasteful  
what if we could share the allocator in vector to the internal strings?




suppose we implement `ctx::vector` and `ctx::string`  
both expect an allocator passed in as param  
suppose also we implement `ctx::bind` that owns an allocator and passes it  
consider:
```cpp
ctx::bind<ctx::Context<Alloc>, ctx::vector<ctx::string>>> vec = ...;
vec[12]; // what does this return?
// wondering if it should be:
// ctx::bind<ctx::Context<Alloc&>, ctx::string&>
```




`unique_ptr` shows an interesting issue  
`ctx::vector<ctx::unique_ptr>` doesn't work?  
vector has allocaror, unique_ptr wants deleter  
do we need an adapter?  
fuck the design choice of deleters, that sucks  
but should we be able to do that?  
maybe  
context transformer?




^ no idea what that was  
BUT  
we have an idea of a `Context`  
furthermore, want a `Bind`  
`Bind` would just own something, and expose it to the passthrough `Context`  
`Bind<Alloc, LeanString>` should have the same behaviour as `std::string`
