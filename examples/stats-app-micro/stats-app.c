/* -----------------------------------------------------------------------------
 * BASIC VERSION OF STATS-APP APPLICATION
 *
 * Vesna starts only with serial_input process. When LGTC sends "start" command,
 * which is character '>', Vesna will start with stats_process.
 * If root also sends '*' command, set device as root of the network.
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
PROCESS(serial_input_process, "Serial input command");
AUTOSTART_PROCESSES(&serial_input_process);

/*---------------------------------------------------------------------------*/
void STATS_input_command(char *data);
void STATS_set_device_as_root(void);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(stats_process, ev, data)
{
  static struct etimer timer;
  static uint8_t addr[8];
  static uint32_t time_counter = 0;

  PROCESS_BEGIN();

  // Send response back to LGTC so it knows we started stats_process
  printf("> \n");

  // Send app duration to LGTC
  printf("AD %d \n", (APP_DURATION_IN_SEC));

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
      // Send stop command ('=') to LGTC
      printf("= \n");
      PROCESS_EXIT();
    }

    // Wait for the periodic timer to expire and then restart the timer
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

    time_counter++;
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(serial_input_process, ev, data)
{
    PROCESS_BEGIN();
    while(1){
      PROCESS_WAIT_EVENT_UNTIL(
        (ev == serial_line_event_message) && (data != NULL));
      STATS_input_command(data);
    }
    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void
STATS_input_command(char *data){
    char cmd = data[0];
    switch(cmd){

		case '>':
        process_start(&stats_process, NULL);
        break;

      case '*':
        STATS_set_device_as_root();
        break;

      case '=':
        // Confirm received stop command
        printf("= \n");
        // TODO: exit root state
        process_exit(&stats_process);
        break;

	  default:
	  //printf("Unknown cmd \n");
	  	break;
    }
}

/*---------------------------------------------------------------------------*/
void
STATS_set_device_as_root(void){
	static uip_ipaddr_t prefix;
	const uip_ipaddr_t *default_prefix = uip_ds6_default_prefix();
	uip_ip6addr_copy(&prefix, default_prefix);

  	if(!NETSTACK_ROUTING.node_is_root()) {
     	NETSTACK_ROUTING.root_set_prefix(&prefix, NULL);
     	NETSTACK_ROUTING.root_start();
	} else {
      	printf("Node is already a DAG root\n");
    }
}
