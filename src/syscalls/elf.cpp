#include <cstddef>
#include <cstdint>
#include <iostream>

// struct magic_number {
//   char data[4] = {0x7f, 'E', 'L', 'F'};
// };

// enum class format : char { BIT32 = 1, BIT64 = 2 };

// enum class endian : char { LITTLE = 1, BIG = 2 };

/*

  https://refspecs.linuxbase.org/elf/elf.pdf

  elf format:
  elf header
  program header table
  segment 1
  segment 2
  ...
  segment N
  section header table (optional)

 */

struct e_ident_t {
  char ei_mag[4]{0x7F, 'E', 'L', 'F'};
  char ei_class{2}; // 64bit
  char ei_data{1};  // little endian
  char ei_version{1};
  char ei_osabi{0}; // no extensions, linux == 3
  char ei_abiversion{0};
  char ei_pad[7]{};
};
static_assert(sizeof(e_ident_t) == 16);

struct elf_header {
  // std::byte magic_number[4]{std::byte{0x7f}, std::byte{'E'}, std::byte{'L'}, std::byte{'F'}};
  // std::byte format{2}; // 64bit
  // std::byte endian{1}; // little
  // std::byte version{1};
  // std::byte os_abi{3}; // linux
  // std::byte padding[8]{};
  e_ident_t e_ident{};
  uint16_t  e_type{2};       // executable
  uint16_t  e_machine{0x3E}; // amd64
  uint32_t  e_version{1};
  void*     e_entry;    // TODO
  uint64_t  e_phoff;    // TODO
  uint64_t  e_shoff;    // TODO
  uint32_t  e_flags{0}; // ???
  uint16_t  e_ehsize{64};
  uint16_t  e_phentsize; // TODO
  uint16_t  e_phnum;     // TODO
  uint16_t  e_shentsize; // TODO
  uint16_t  e_shnum;     // TODO
  uint16_t  e_shstrndx;  // TODO
};
static_assert(sizeof(elf_header) == 64);

struct section_header {
  uint32_t sh_name;
  uint32_t sh_type;
  uint64_t sh_flags;
  void*    sh_addr;
  uint64_t sh_offset;
  uint64_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
};

struct program_header {
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  void*    p_vaddr;
  void*    p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
};

int main() {
  std::string code;
  {
    /*
      0:	48 8b 04 25 e7 00 00 	mov    0xe7,%rax
      7:	00
      8:	48 8b 3c 25 7b 00 00 	mov    0x7b,%rdi
      f:	00
      10:	0f 05                	syscall

      WRONG

      0:	48 c7 c0 e7 00 00 00 	mov    $0xe7,%rax
      7:	48 c7 c7 7b 00 00 00 	mov    $0x7b,%rdi
      e:	0f 05                	syscall
     */

    auto        decode = [](char c) { return c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10; };
    std::string rep    = "48 c7 c0 e7 00 00 00 48 c7 c7 7b 00 00 00 0f 05";
    for (size_t i = 0; i < rep.size(); i += 3) {
      code.push_back((char)(decode(rep[i]) * 16 + decode(rep[i + 1])));
    }
  }

  {
    elf_header x{};
    x.e_entry     = reinterpret_cast<void*>(sizeof(elf_header) + sizeof(program_header) + 65536);
    x.e_phoff     = sizeof(x);
    x.e_shoff     = 0;
    x.e_phentsize = sizeof(program_header);
    x.e_phnum     = 1;
    x.e_shentsize = 0; // sizeof(section_header);
    x.e_shnum     = 0;
    x.e_shstrndx  = 0;

    std::string_view sv{(char*)&x, sizeof(x)};
    std::cout << sv;
  }

  {
    program_header x;
    x.p_type   = 1;
    x.p_flags  = 5; // XR
    x.p_offset = sizeof(elf_header) + sizeof(program_header);
    x.p_vaddr  = reinterpret_cast<void*>(x.p_offset + 65536);
    x.p_paddr  = x.p_vaddr;
    x.p_filesz = code.size(); // 2;
    x.p_memsz  = 0x1000;      // code.size();
    x.p_align  = 1;

    std::string_view sv{(char*)&x, sizeof(x)};
    std::cout << sv;
  }

  {
    // std::cout << (char)(0x0F) << (char)(0xFF);
    std::cout << code;
  }
}
