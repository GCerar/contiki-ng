/* -----------------------------------------------------------------------------
 * MINIATURE VERSION OF STATS-APP APPLICATION - ONLY MONITORING, NO RESPONSE
 *
 * Vesna starts with stats_process and prints Hello every second for 1h (default)
 * LGTC doesn't check if vesna is responding, it just stores what it gets.
 * 
 * Make sure to enter the same time (APP_DURATION_IN_SEC) here and in 
 * serial_monitor.py (MAX_APP_TIME)! 
 * -----------------------------------------------------------------------------
*/

#include <stdio.h>
#include "contiki.h"
#include "net/ipv6/uip.h"
#include "dev/serial-line.h"
#include "arch/platform/vesna/dev/at86rf2xx/rf2xx_stats.h"

/*---------------------------------------------------------------------------*/
#define APP_DURATION_IN_SEC    (60 * 60)

/*---------------------------------------------------------------------------*/
PROCESS(stats_process, "Stats process");
AUTOSTART_PROCESSES(&stats_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(stats_process, ev, data)
{
  static struct etimer timer;
  static uint8_t addr[8];
  static uint32_t time_counter = 0;

  PROCESS_BEGIN();

  // Send IPv6 address of Vesna to LGTC
  rf2xx_driver.get_object(RADIO_PARAM_64BIT_ADDR, &addr, 8);
	printf("Device IP: ");
	for(int j=0; j<8; j++){
		printf("%X",addr[j]);
	}
  printf("\n");

  // Setup a periodic timer that expires after 1 second
  etimer_set(&timer, CLOCK_SECOND );

  time_counter = 0;

  while(1) {

    printf("Hello!\n");

    if(time_counter == APP_DURATION_IN_SEC) {
      PROCESS_EXIT();
    }

    // Wait for the periodic timer to expire and then restart the timer
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

    time_counter++;
  }

  PROCESS_END();
}