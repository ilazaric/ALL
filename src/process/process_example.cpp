#include <ivl/process>

int main() {
  ivl::process_config cfg;
  cfg.pathname = "/bin/touch";
  cfg.argv     = {"/bin/touch", "/home/ilazaric/repos/ALL/src/process/evidence"};
  auto proc    = cfg.clone_and_exec();
}
