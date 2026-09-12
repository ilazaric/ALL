extern int foo(void);

extern int puts(const char*);
extern void *dlopen(const char *filename, int flags);
extern char *dlerror(void);
extern int dlclose(void *handle);
extern int dlclose_forced(void *handle);

#define RTLD_LAZY 0x1
#define RTLD_NOW 0x2
#define RTLD_GLOBAL 0x00100

int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    puts(argv[i]);
    void* h = dlopen(argv[i], RTLD_NOW | RTLD_GLOBAL);
    if (!h) {
      puts(dlerror());
      return 9;
    }
    int x = foo();
    char s[10] = {0};
    int l = 0;
    while (x) {
      s[l++] = '0' + x % 10;
      x /= 10;
    }
    if (l == 0) s[l++] = '0';
    for (int j = 0; j*2 < l; ++j) {
      char c = s[j];
      s[j] = s[l-j-1];
      s[l-j-1] = c;
    }
    puts(s);
    if (dlclose_forced(h)) {
      puts(dlerror());
      return 9;
    }
  }
}
