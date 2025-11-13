#define PRINT_BUF_SIZE 1024

struct console
{
  char init;
  volatile char in_char;
};

extern struct console * const console;

void console_init( void );
void console_putc( char ch );
void console_write( char const *string );
void console_printf( char const *fmt, ... );

