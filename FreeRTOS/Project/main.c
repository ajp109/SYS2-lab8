/* Standard includes. */

/* Local includes. */
#include "console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

static void prvTask1( void * pvParameters ) {
  while(1) {
    terminal_print("Hello from Task 1\n");
  }
}

static void prvTask2( void * pvParameters ) {
  while(1) {
    terminal_print("This is a message from Task 2\n");
  }
}

void main( void )
{
  xTaskCreate( prvTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( prvTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  terminal_print("Program starting...\n");
  vTaskStartScheduler();
}

