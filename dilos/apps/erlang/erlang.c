#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <limits.h>

#define ARGS_FILE "/etc/beam.args"
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define ROOTDIR "/usr/lib64/erlang"
#define BINDIR ROOTDIR "/erts-" STR(ERTS_VERSION) "/bin"

int main(int argc, char **argv){
  setenv("ROOTDIR", ROOTDIR, false);
  setenv("BINDIR", BINDIR, false);
  setenv("TERM", "vt100-qemu", false);

  while(--argc) {
    argv++;

    if (strcmp(argv[0], "-env"))
        break;

    argc -= 2;
    if (argc < 0) {
      fprintf(stderr, "too few argument for env\n");
      exit(1);
    }

    setenv(argv[1], argv[2], true);
    argv += 2;
  }

  if (argc != 2) {
    fprintf(stderr, "exactly two arguments required\n");
    exit(1);
  }

  char *args_file = argv[0];
  char *fallback = argv[1];

  remove(ARGS_FILE);
  rename(args_file, ARGS_FILE);
  link(fallback, ARGS_FILE);

  char path[PATH_MAX] = {0};
  snprintf(path, PATH_MAX, "%s/erlexec", BINDIR);

  void *elf_handle = dlopen(path, RTLD_LAZY);

  if (!elf_handle)
    goto err;

  int (*mainfun)(int, char **) = (int (*)(int, char **)) dlsym(elf_handle, "main");

  char *erl_argv[] = {"erlexec", "-args_file", ARGS_FILE, NULL};
  int result = (mainfun)?mainfun(3, erl_argv):1;
  dlclose(elf_handle);

  if(!mainfun)
    goto err;

  return result;
err:
  fprintf(stderr, "Error spawning erlexec\n");
  return 1;
}
