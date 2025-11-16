#include "console.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

static bool irqEnabled = false;

/* IRQ enable and disable */
void console_IRQ_enable(console_IRQ_set_t set)
{
  irqEnabled = ( ( set & IRQ_KEYBOARD ) != 0 );
}
void console_IRQ_disable(console_IRQ_set_t set)
{
  irqEnabled = ( ( set & IRQ_KEYBOARD ) == 0 );
}

/* Default "ISR", override to handle */
__attribute(( weak )) void console_keyboard_ISR( void ) {
  dprintf(2, "Default ISR triggered\n");
  while( 1 );
}

/* Console configuration container, including received key */
static struct console _console = { .init = false };
struct console * const console = &_console;

/* Wrapped putc implementation. */
void console_putc( char ch )
{
  if ( !console->init ) {
    dprintf( 2, "ERROR: Console not initialised\n" );
  } else {
    while( write( 1, &ch, 1 ) <= 0 );
    fdatasync( 1 );
  }
}

/* Less thread-safe write() to stdout. */
void console_write( char const *string )
{
  while( *string ) {
    console_putc( *(string++) );
  }  
}

/* Less thread-safe printf() replacement, to more accurately simulate
 * embedded print implementations.  Interruptible between characters
 * Static buffer ensures that races will cause print corruption. */
void console_printf( char const *fmt, ... )
{
  static char buffer[ PRINT_BUF_SIZE ] = { 0 };
  va_list vargs;
  va_start( vargs, fmt );
  size_t n = vsnprintf( buffer, PRINT_BUF_SIZE - 1, fmt, vargs );
  console_write( buffer );
  va_end( vargs );
}

/* Code to clean up terminal settings changes */
static struct termios oldt;
static void cleanup( void )
{
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
}
static void sig_handler( int signum )
{
  cleanup( );
  struct sigaction sa;
  sa.sa_handler = SIG_DFL;
  sigemptyset( &sa.sa_mask );
  sigaction( signum, &sa, NULL );
  raise( signum );
}

/* Signal handler for keystrokes, dispatched to a task thread */
static void kb_handler( int signum )
{
  ( void ) signum;
  vPortDisableInterrupts( );
  console_keyboard_ISR( );
  vPortEnableInterrupts( );
}

/*
 * The additional per-thread data is stored at the beginning of the
 * task's stack. The pthread is at the beginning of that.
 */
static inline pthread_t prvGetThreadFromTask( TaskHandle_t xTask )
{
    StackType_t * pxTopOfStack = *( StackType_t ** ) xTask;

    return * ( ( pthread_t * ) ( pxTopOfStack + 1 ) );
}

/* Kernel hack to avoid preemption when firing signal */
extern BaseType_t volatile pdTickDisabled;

/* Mimic an interrupt handler, firing a callback whenever a key is struck */
void *keyboardThreadFn( void *arg )
{
  ( void ) arg;
  
  sigset_t set;
  sigfillset( &set );
  pthread_sigmask( SIG_SETMASK, &set, NULL );
  
  ssize_t status;
  while( 1 )
  {
    status = read( 0, ( void * ) &( console->in_char ), 1 );
    if( status < 0 && errno == EINTR )
    {
      continue;
    }
    if( status <= 0 )
    {
      break;
    }

    if( irqEnabled )
    {
      pdTickDisabled = pdTRUE;
      pthread_t thread = prvGetThreadFromTask( xTaskGetCurrentTaskHandle() );
      pthread_kill( thread, SIGUSR2 );
      pdTickDisabled = pdFALSE;
    }
  }

  if( status != 0 )
  {
    dprintf( 2, "Read returned %jd\n", status );
    exit( 1 );
  }
  exit( 0 );
}


/* Initialise console IRQ simulation */
void console_init( void )
{
  /* Unbuffer stdin, remove echo
   * Undone by cleanup(), called at clean exit
   * or by SIGINT, SIGHUP, SIGQUIT, SIGSEGV, or SIGTERM handlers */
  struct termios newt;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );

  /* Trap most signals for cleanup */
  struct sigaction sa;
  sa.sa_handler = sig_handler;
  sigemptyset( &sa.sa_mask );
  sigaction( SIGINT, &sa, NULL );
  sigaction( SIGHUP, &sa, NULL );
  sigaction( SIGQUIT, &sa, NULL );
  sigaction( SIGSEGV, &sa, NULL );
  sigaction( SIGTERM, &sa, NULL );
  atexit( cleanup );
  struct sigaction sa_kb;
  sa_kb.sa_handler = kb_handler;
  sigemptyset( &sa_kb.sa_mask );
  sigaction( SIGUSR2, &sa_kb, NULL );
  
  pthread_t keyboardThread;
  pthread_create( &keyboardThread, NULL, keyboardThreadFn, NULL );
  
  console->init = true;
}

