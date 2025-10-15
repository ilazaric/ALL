what is a running process?
pid, and a set of shared file descriptors?
by default std{in,out,err}
after termination we also get some stats
not-yet-started -> clone-pre-exec -> running -> dead
^ set up some file descriptors
                   ^ prctl, fix up file descriptors
                                     ^ none
                                                ^ wait4, collect stats
                                                
i could communicate with children via file descriptors
fd 0 -- input file
fd 1 -- output file, maybe opened with O_CREAT
fd 2 -- stderr pipe

compile_cxx(input, output)
compile_cxx(input) -> output pipe
compile_cxx() <- input pipe -> output pipe
run_python(input)

set up some pipes
run processes

