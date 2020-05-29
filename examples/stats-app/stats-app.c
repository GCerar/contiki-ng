
#include <stdio.h> /* For printf() */
#include "contiki.h"
#include "net/ipv6/uip.h"
#include "dev/serial-line.h"



// TODO: Gregor help?
//#include "arch/platform/vesna/dev/at86rf2xx/rf2xx.h"
#include "arch/platform/vesna/dev/at86rf2xx/rf2xx_stats.h"

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(serial_input_process, "Serial input command");
AUTOSTART_PROCESSES(&serial_input_process);
/*---------------------------------------------------------------------------*/

uint32_t counter = 0;

void STATS_input_command(char *data);
void STATS_set_device_as_root(void);

PROCESS_THREAD(hello_world_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  printf("> \n");

  printf("AD %d \n", (60*200));

	counter = 0;
	
  /* Setup a periodic timer that expires after 10 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 10);

  while(1) {
    printf("Still alive\n");

	counter++;
	if(counter == (6 * 200)){
		printf("End of app-time \n")
		printf("= \n");
		PROCESS_EXIT();
	}

    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

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

void
STATS_input_command(char *data){
    char cmd = data[0];
    switch(cmd){

		case '>':
        process_start(&hello_world_process, NULL);
        break;

      case '*':
        STATS_set_device_as_root();
        break;

	  default:
	  //printf("Unknown cmd \n");
	  	break;
    }
}

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