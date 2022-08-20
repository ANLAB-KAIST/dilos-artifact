//
// Copyright (C) 2018 Waldek Kozaczuk
//
// This work is open source software, licensed under the terms of the
// BSD license as described in the LICENSE file in the top-level directory.
//
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
   MonoDomain *domain;
   MonoAssembly *assembly;
   char *program_name;
   int retval;

   if (argc < 2) {
      printf("Usage: mono_exec <exe_file_name>\n");
      exit(1);
   }

   program_name = argv[1];
   domain = mono_jit_init(program_name);

   assembly = mono_domain_assembly_open(domain, program_name);
   if (!assembly) {
      printf("Failed to open assembly: %s\n", program_name);
      exit(2);
   }

   retval = mono_jit_exec(domain, assembly, argc - 1, argv + 1);
   mono_jit_cleanup(domain);

   return retval;
}
