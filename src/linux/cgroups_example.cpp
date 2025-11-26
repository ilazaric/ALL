#include <ivl/linux/cgroups>
#include <boost/asio.hpp>
#include <cstdio>
#include <cstring>
#include <ivl/stl/string>

// IVL add_compiler_flags("-static -flto")
// IVL add_compiler_flags_tail("-pthread -lboost_system")

int main() {
  using namespace ivl::linux::cgroups;

  std::filesystem::path topcg = "distrib";
  auto workercg1 = topcg / "worker1";
  auto workercg2 = topcg / "worker2";

  auto write = [](auto cg, auto file, auto txt) {
    auto ff = cgroup_dir(cg) / file;
    auto f = std::fopen(ff.native().c_str(), "w");
    assert(f);
    std::println(f, "{}", txt);
    auto r = std::fclose(f);
    if (r != 0) {
      auto e = errno;
      std::println("{} {} {}", cg.native(), file, txt);
      std::println("err: {}", std::strerror(e));
      std::println("ff: {}", ff.native());
      assert(false);
    }
  };

  if (exists(cgroup_dir(workercg1))) destroy(workercg1);
  if (exists(cgroup_dir(workercg2))) destroy(workercg2);
  if (exists(cgroup_dir(topcg))) destroy(topcg);

  auto create_init = [&](auto cg, auto memmax, auto cpumax) {
    create(cg);
    write(cg, "cgroup.subtree_control", "+cpu +memory");
    write(cg, "memory.max", memmax);
    write(cg, "cpu.max", cpumax);
  };

  create_init(topcg, "500M", "200000 100000");
  create_init(workercg1, "100M", "100000 100000");
  create_init(workercg2, "100M", "100000 100000");

  // try {
  //   using boost::asio::ip::tcp;
  //   boost::asio::io_context io;
  //   tcp::endpoint endpoint(tcp::v4(), 6767);
  //   tcp::acceptor acceptor(io, endpoint);
  //   size_t next_conn_idx = 0;
  //   struct conn_state {
  //     tcp::socket socket;
  //     char buf[1024];
  //     std::string partial;
  //   };
  //   std::unordered_map<size_t, conn_state> connections;
  //   auto exec_command = [&](conn_state* conn, std::string_view command) {
  //     auto parts = ivl::split_py(command);
  //     if (parts[0] == "setmem"){
  //       if (parts.size() != 3) return;
  //       auto worker_idx = boost::lexical_cast<int>(parts[1]);
  //       auto mem = parts[2];
  //       write(worker_idx == 1 ? workercg1 : workercg2, "memory.max", mem);
  //     } else if (parts[0] == "setcpu"){
  //       if (parts.size() != 3) return;
  //       auto worker_idx = boost::lexical_cast<int>(parts[1]);
  //       auto cpu = parts[2];
  //       write(worker_idx == 1 ? workercg1 : workercg2, "cpu.max", cpu + " 100000");
  //     } else if (parts[0] == "exec"){
  //       // TODO
  //     }
  //   };
  //   auto accept_lambda = [&](this auto&& self, boost::system::error_code ec, tcp::socket socket) {
  //     acceptor.async_accept(self);
  //     if (ec) return;
  //     idx = next_conn_idx++;
  //     auto conn_ptr = &(connections.try_emplace(idx, conn_state{std::move(socket)}).first->second);
  //     conn_ptr->async_read_some([&](this auto&& self, boost::system::error_code ec, std::size_t n) {
  //       if (ec) {
  //         conn_ptr->socket.shutdown(tcp::shutdown_both, ec);
  //         conn_ptr->socket.close(ec);
  //         return;
  //       }
  //       conn_ptr->socket.async_read_some(self);
  //       std::string_view data(conn_ptr->buf, n);
  //       while (true) {
  //         auto loc = data.find('\n');
  //         if (loc == std::string_view::npos) break;
  //         // TODO
  //         if (conn_ptr->partial.empty()) exec_command(conn_ptr, data.substr(0, loc));
  //         else {
  //           exec_command(conn_ptr, conn_ptr->partial + data.substr(0, loc));
  //           conn_ptr->partial.clear();
  //         }
  //         data = data.substr(loc + 1);
  //       }
  //       conn_ptr->partial += data;
  //     });
  //   };
  //   acceptor.async_accept(accept_lambda);
  //   io.run();
  // } catch () {
  // }
}
