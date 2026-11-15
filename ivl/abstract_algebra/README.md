how to do this?  
important to get perf good  

an obvious choice would be `operator*()`  
but this makes a runtime-specified structure annoying  
maybe do the thing you wanted for allocators  
the context passing thing  

so we have two types  
`Group`  
`GroupElement`  
`Group::multiply(const GroupElement&, const GroupElement&)`  
`GroupInterface::multiply(Group&, const GroupElement&, const GroupElement&)`  
probably dont need last one yet  
though ...  
`GroupInterface::multiply_assign()` could fallback onto `multiply()`  
whatever for now  

would also need EBO for `Group` usually tbh  
David Stone save me  
(opt constexpr)  


so how would structures containing group elements look?  
we would want to query them as well  
return of the context structure?  
