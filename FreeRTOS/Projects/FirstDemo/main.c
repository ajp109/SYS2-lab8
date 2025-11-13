/* Standard includes. */

/* Local includes. */
#include "console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t mutex;

void console_ISR() {
  console_write(".This is a long message which I hope will never be interrupted.\n");
}

static void prvTickTask( void * pvParameters ) {
  while( 1 )
  {
    console_putc('.');
    vTaskDelay(1);
  }
}

static void prvTask1( void * pvParameters ) {
  while( 1 )
  {
    vTaskDelay(10);
    xSemaphoreTake(mutex, portMAX_DELAY);
    console_printf( "(low priority)\n" );
    vTaskDelay(9);
    xSemaphoreGive(mutex);
  }
}

static void prvTask2( void * pvParameters ) {
  while( 1 )
  {
    vTaskDelay(9);
//    printf("2w\n");
    xSemaphoreTake(mutex, portMAX_DELAY);
    console_printf( "!!! HIGH PRIORITY !!!\n" );
    xSemaphoreGive(mutex);
  }
}

static void prvTask3( void * pvParameters ) {
  while( 1 ) {
    vTaskDelay(100);
//    printf("%d\n",uxSemaphoreGetCount(mutex));
    for(volatile uint64_t i=0;i<1e8;++i);
  }
}

void main( void ) {
  console_init( );
  mutex = xSemaphoreCreateBinary();
  xSemaphoreGive(mutex);
  xTaskCreate( prvTickTask, "Tick", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + configMAX_PRIORITIES, NULL );
  xTaskCreate( prvTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( prvTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL );
  xTaskCreate( prvTask3, "Task 3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL );
  console_printf( "Program starting...\n" );
  vTaskStartScheduler( );
}

