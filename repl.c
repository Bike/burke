#include <stdio.h>
#include <setjmp.h>
#include <string.h> // strcmp
#include <stdlib.h> // exit
#include "read.h"
#include "write.h"
#include "lisp.h"
#include "error.h"
#include "package.h"
#include "alloc.h"
#include "types.h"
#include "port.h"

jmp_buf err_jmp;

lispobj* jump_with_eof(const char* format, ...) {
  va_list vargs;
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  putc('\n', stderr);
  va_end(vargs);
  if (!strcmp(format, "unexpected EOF")) // ugh
    exit(1);
  longjmp(err_jmp, 0);
}

int main(void) {
  initialize_globals(); // nil etc., same across threads
  // burke_state = initialize_state(); // "dynamic binding" through TLS
  lispobj* package_o = make_package(100);
  lisp_package* package = LO_GET(lisp_package, *package_o);
  lispobj* ground = make_ground(package_o);
  lispobj* lstdout = make_port(stdout);
  lerror = jump_with_eof; // set up error handler

  define(find_or_intern("stdin", package), make_port(stdin), ground);
  define(find_or_intern("stdout", package), lstdout, ground);
  define(find_or_intern("stderr", package), make_port(stderr), ground);

  setjmp(err_jmp); // don't care about return
  while(1) {
    write_lisp(eval(read_lisp(stdin, package), ground), lstdout);
    putchar('\n');
    fflush(stdout);
  }
}
