#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

static char* get_start(){
  int fd;
  char* ptr;
  fd = open("/dev/myreservedmem", O_RDWR);
  assert(fd != -1);
  ptr = mmap(0, 1ULL<<29, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  assert(ptr != 0);
  return ptr;
}

static char* remaining_memory = 0;
static size_t remaining_size = 1ULL<<29;

void *malloc(size_t size){
  void* ret;
  if (size == 0) return 0;
  if (remaining_memory == 0) remaining_memory = get_start();
  assert(remaining_size >= size);
  remaining_size -= size;
  ret = remaining_memory;
  remaining_memory += size;
  return (void*)ret;
}
  
void free(void * ptr){
}
