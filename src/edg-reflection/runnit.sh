

export EDG_DECODE_PATH =
  / home / ilazaric / repos / ALL / src / edg -
  reflection / cc / bin / edg_decode export EDG_RUNTIME_LIB = edgrt export EDG_GCC_INCL_SCRAPE =
    / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib / gcc / current / gcc /
      x86_64 -
    pc - linux -
    gnu / 14 /../../../../../../ include / c++ /
      14 : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib / gcc / current /
           gcc / x86_64 -
    pc - linux - gnu / 14 /../../../../../../ include / c++ / 14 / x86_64 - pc - linux -
    gnu : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib / gcc / current /
          gcc / x86_64 -
    pc - linux -
    gnu / 14 /../../../../../../ include / c++ / 14 /
      backward : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib / gcc /
                 current / gcc / x86_64 -
    pc - linux -
    gnu / 14 / include : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib /
                         gcc / current / gcc / x86_64 -
    pc - linux - gnu / 14 / include - fixed / x86_64 - linux -
    gnu : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib / gcc / current /
          gcc / x86_64 -
    pc - linux - gnu / 14 / include -
    fixed : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib / gcc /
            current / gcc /../../../../ include : / home / linuxbrew /.linuxbrew /
                                                  include : / usr / include / x86_64 -
    linux - gnu : / usr / include export ECCP_LIBDIR =
      / home / ilazaric / repos / ALL / src / edg - reflection / cc / lib export EDG_BASE =
        / home / ilazaric / repos / ALL / src / edg -
        reflection / cc / base export EDG_C_TO_OBJ_COMPILER =
          / home / linuxbrew /.linuxbrew / bin / gcc - 14 export ECCP =
            / home / ilazaric / repos / ALL / src / edg -
            reflection / cc / bin / eccp export EDG_OBJ_TO_EXEC_DEFAULT_OPTIONS =
              "-static -z muldefs" export EDG_CPFE_DEFAULT_OPTIONS =
                "--gnu 140200 --c++26 --diag_suppress last_line_incomplete --no_wrap_diagnostics "
                "--exceptions" export CPFE =
                  / home / ilazaric / repos / ALL / src / edg -
                  reflection / cc / bin / cpfe export EDG_C_TO_OBJ_DEFAULT_OPTIONS =
                    "-w -Dva_copy=__va_copy -falign-functions=4" export EDG_PRELINK_PATH =
                      / home / ilazaric / repos / ALL / src / edg -
                      reflection / cc / bin / edg_prelink export EDG_GCC_CINCL_SCRAPE =
                        / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../ lib /
                          gcc / current / gcc / x86_64 -
                        pc - linux -
                        gnu / 14 / include : / home / linuxbrew /.linuxbrew / Cellar / gcc /
                                             14.2.0_1 / bin /../ lib / gcc / current / gcc /
                                             x86_64 -
                        pc - linux - gnu / 14 / include - fixed / x86_64 - linux -
                        gnu : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../
                              lib / gcc / current / gcc / x86_64 -
                        pc - linux - gnu / 14 / include -
                        fixed : / home / linuxbrew /.linuxbrew / Cellar / gcc / 14.2.0_1 / bin /../
                                lib / gcc / current / gcc /../../../../
                                include : / home / linuxbrew /.linuxbrew /
                                          include : / usr / include / x86_64 -
                        linux - gnu : / usr / include export EDG_INSTALL_DIR =
                          / home / ilazaric / repos / ALL / src / edg -
                          reflection / cc export EDG_MUNCH_PATH =
                            / home / ilazaric / repos / ALL / src / edg -
                            reflection / cc / bin /
                              edg_munch

                                gdb-- args /
                              home / ilazaric / repos / ALL / src / edg -
                            reflection / cc / bin / cpfe - D_POSIX_SOURCE -
                            D__CHAR_BIT__ = 8 --max_depth_constexpr_call = 1000000 --sys_include =
                              "/home/ilazaric/repos/ALL/src/edg-reflection/cc/base/"
                              "include_cpp_exp/" --sys_include =
                                "/home/linuxbrew/.linuxbrew/Cellar/gcc/14.2.0_1/bin/../lib/gcc/"
                                "current/gcc/x86_64-pc-linux-gnu/14/../../../../../../include/c++/"
                                "14" --sys_include =
                                  "/home/linuxbrew/.linuxbrew/Cellar/gcc/14.2.0_1/bin/../lib/gcc/"
                                  "current/gcc/x86_64-pc-linux-gnu/14/../../../../../../include/"
                                  "c++/14/x86_64-pc-linux-gnu" --sys_include =
                                    "/home/linuxbrew/.linuxbrew/Cellar/gcc/14.2.0_1/bin/../lib/gcc/"
                                    "current/gcc/x86_64-pc-linux-gnu/14/../../../../../../include/"
                                    "c++/14/backward" --sys_include =
                                      "/home/linuxbrew/.linuxbrew/Cellar/gcc/14.2.0_1/bin/../lib/"
                                      "gcc/current/gcc/x86_64-pc-linux-gnu/14/"
                                      "include" --sys_include =
                                        "/home/linuxbrew/.linuxbrew/Cellar/gcc/14.2.0_1/bin/../lib/"
                                        "gcc/current/gcc/x86_64-pc-linux-gnu/14/include-fixed/"
                                        "x86_64-linux-gnu" --sys_include =
                                          "/home/linuxbrew/.linuxbrew/Cellar/gcc/14.2.0_1/bin/../"
                                          "lib/gcc/current/gcc/x86_64-pc-linux-gnu/14/"
                                          "include-fixed" --sys_include =
                                            "/home/linuxbrew/.linuxbrew/Cellar/gcc/14.2.0_1/bin/../"
                                            "lib/gcc/current/gcc/../../../../"
                                            "include" --sys_include =
                                              "/home/linuxbrew/.linuxbrew/include" --sys_include =
                                                "/usr/include/x86_64-linux-gnu" --sys_include =
                                                  "/usr/include" - tused-- gen_c_file_name =
                                                    / tmp / eccp248834 / init -
                                                    exp.rendered.int
                                                      .c-- gnu 140200 --c++ 26 --diag_suppress
                                                        last_line_incomplete-- no_wrap_diagnostics-- exceptions
                                                          init -
                                                    exp.rendered.cpp
