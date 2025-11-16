/* Standard includes. */

/* Local includes. */
#include "console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t mutex;

typedef struct {
  char const * message;
  TickType_t delay;
} TaskData_t;

static void prvPrintTask( void * pvParameters ) {
  TaskData_t const * const data = pvParameters;
  while( 1 )
  {
    vTaskDelay( data->delay );
    xSemaphoreTake( mutex, portMAX_DELAY );
    console_write( data->message );
    xSemaphoreGive( mutex );
  }
}

void main( void )
{
  console_init( );
  mutex = xSemaphoreCreateMutex( );
  TaskData_t taskData1 = { .message = "Message from Task 1\n", .delay = 250 };
  TaskData_t taskData2 = { .message = "Hello from Task 2\n", .delay = 600 };
  xTaskCreate( prvPrintTask, "Task 1", configMINIMAL_STACK_SIZE, &taskData1, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( prvPrintTask, "Task 2", configMINIMAL_STACK_SIZE, &taskData2, tskIDLE_PRIORITY + 1, NULL );
  console_printf( "Program starting...\n" );
  vTaskStartScheduler( );
}

