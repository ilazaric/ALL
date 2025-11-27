#include <ivl/process>

// IVL add_compiler_flags("")

struct safe_process_config {
  ivl::process_config                raw;
  size_t                             memory_limit;
  float                              cpu_limit;
  float                              time_limit;
  std::vector<std::filesystem::path> input_files;
  std::vector<std::filesystem::path> output_files;
  std::filesystem::path              tmp_dir; // in ramfs
};

int main() {
  ivl::process_config cfg;
  cfg.pathname = "/bin/touch";
  cfg.argv     = {"/bin/touch", "/home/ilazaric/repos/ALL/src/process/evidence"};
  auto proc    = cfg.clone_and_exec();
}
