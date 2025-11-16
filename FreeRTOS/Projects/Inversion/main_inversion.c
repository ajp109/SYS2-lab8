/* Standard includes. */

/* Local includes. */
#include "console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t mutex;

static void prvTickTask( void * pvParameters ) {
  while( 1 )
  {
    console_putc('.');
    vTaskDelay(1);
  }
}

static void prvLowTask( void * pvParameters ) {
  while( 1 )
  {
    vTaskDelay( 10 );
    xSemaphoreTake( mutex, portMAX_DELAY );
    console_printf( "(low priority)\n" );
    vTaskDelay( 9 );
    xSemaphoreGive( mutex );
  }
}

static void prvMidTask( void * pvParameters ) {
  while( 1 ) {
    vTaskDelay( 100 );
    for( volatile uint64_t i=0; i<1e8; ++i );
  }
}

static void prvHighTask( void * pvParameters ) {
  while( 1 )
  {
    vTaskDelay(9);
    xSemaphoreTake( mutex, portMAX_DELAY );
    console_printf( "!!! HIGH PRIORITY !!!\n" );
    xSemaphoreGive( mutex );
  }
}

void main( void )
{
  console_init( );
  mutex = xSemaphoreCreateBinary();
  xSemaphoreGive(mutex);
  xTaskCreate( prvTickTask, "Tick", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + configMAX_PRIORITIES, NULL );
  xTaskCreate( prvLowTask, "Low", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( prvMidTask, "Medium", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL );
  xTaskCreate( prvHighTask, "High", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL );
  console_printf( "Program starting...\n" );
  vTaskStartScheduler( );
}

