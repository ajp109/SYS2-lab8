#include "console.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

void terminal_print( char const *fmt, ... ) {
  static char buffer[ PRINT_BUF_SIZE ] = { 0 };
  va_list vargs;
  va_start( vargs, fmt );
  size_t n = vsnprintf( buffer, PRINT_BUF_SIZE-1, fmt, vargs );
  char *chr = buffer;
  while (*chr) {
    if (write(1, chr, 1) > 0) ++chr;
    fdatasync(1);
  }
  va_end( vargs );
}

