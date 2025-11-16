#include <stdbool.h>

#define PRINT_BUF_SIZE 1024
#define IRQ_KEYBOARD ( 1UL )

struct console
{
  bool init;
  volatile char in_char;
};

extern struct console * const console;

typedef int console_IRQ_set_t;
void console_IRQ_enable(console_IRQ_set_t set);
void console_IRQ_disable(console_IRQ_set_t set);

void console_init( void );
void console_putc( char ch );
void console_write( char const *string );
void console_printf( char const *fmt, ... );

