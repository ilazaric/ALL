set pagination off

set logging file open_cookies.gdb.output
set logging overwrite on
set logging enabled on

set breakpoint pending on

set detach-on-fork off
set schedule-multiple on
set non-stop on

# condition 1 $_streq((const char*)$rsi, "/home/ilazaric/repos/ALL/ivl/experiments/scrapingcourse/uc-cookies/foo/Default/Cookies")

# catch syscall openat
# command
# backtrace
# continue
# end

# catch syscall open
# command
# backtrace
# continue
# end

# catch syscall openat2
# command
# backtrace
# continue
# end

# catch syscall creat
# command
# backtrace
# continue
# end

break open if $_streq((const char*)$rdi, "/home/ilazaric/repos/ALL/ivl/experiments/scrapingcourse/uc-cookies/foo/Default/Cookies")
command
backtrace
continue
end

break creat if $_streq((const char*)$rdi, "/home/ilazaric/repos/ALL/ivl/experiments/scrapingcourse/uc-cookies/foo/Default/Cookies")
command
backtrace
continue
end

break openat if $_streq((const char*)$rsi, "/home/ilazaric/repos/ALL/ivl/experiments/scrapingcourse/uc-cookies/foo/Default/Cookies")
command
backtrace
continue
end

break openat2 if $_streq((const char*)$rsi, "/home/ilazaric/repos/ALL/ivl/experiments/scrapingcourse/uc-cookies/foo/Default/Cookies")
command
backtrace
continue
end

run&

# set logging enabled off
# quit
