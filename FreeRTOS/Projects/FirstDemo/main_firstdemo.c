/* Standard includes. */

/* Local includes. */
#include "console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

static void prvTask1( void * pvParameters ) {
  while( 1 ) {
    console_printf( "Hello from Task 1\n" );
  }
}

void main( void ) {
  console_init( );
  xTaskCreate( prvTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  console_printf( "Program starting...\n" );
  vTaskStartScheduler( );
}

