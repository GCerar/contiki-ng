/* -----------------------------------------------------------------------------
 * STATS-APP for MMT-IOT Fed4FIRE+ experiment
 * -----------------------------------------------------------------------------
*/

#include "contiki.h"
#include <stdio.h>
#include "dev/serial-line.h"
#include "arch/platform/vesna/dev/at86rf2xx/rf2xx.h"
#include "arch/platform/vesna/dev/at86rf2xx/rf2xx_stats.h"
#include "net/ipv6/uip.h"
#include "net/routing/rpl-classic/rpl-private.h"

/*---------------------------------------------------------------------------*/
#define SECOND 		  (1000)
#define MAX_APP_TIME  (60 * 90) // 1:30 

uint32_t counter = 0;

enum STATS_commands {cmd_start, cmd_stop, app_duration};
/*---------------------------------------------------------------------------*/
void STATS_print_help(void);
void STATS_input_command(char *data);
void STATS_output_command(uint8_t cmd);
void STATS_set_device_as_root(void);
void STATS_close_app(void);

/*---------------------------------------------------------------------------*/
PROCESS(stats_process, "Stats app process");
PROCESS(serial_input_process, "Serial input command");

AUTOSTART_PROCESSES(&serial_input_process);

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
        process_start(&stats_process, NULL);
        break;
      
      case '*':
        STATS_set_device_as_root();
        break;
      
      case '=':
        process_exit(&stats_process);
        STATS_close_app();
        break;

	  default:
	  	break;
    }
}

void
STATS_output_command(uint8_t cmd)
{
	switch(cmd){
		case cmd_start:
			printf("> \n");
			break;

		case cmd_stop:
			printf("= \n");
			break;

		case app_duration:
			printf("AD %d\n", (MAX_APP_TIME));
			break;
		
		default:
			printf("Unknown output command \n");
			break;
	}
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(stats_process, ev, data)
{
	static struct etimer timer;

	PROCESS_BEGIN();

	// Respond to LGTC
	STATS_output_command(cmd_start);

	counter = 0;  

	// Empty buffers if they have some values from before
	RF2XX_STATS_RESET();
	STATS_clear_packet_stats();

	// Send app duration to LGTC
	STATS_output_command(app_duration);

	STATS_print_help();

	etimer_set(&timer, SECOND);

	while(1) {
		counter++;	

		if((counter % 10) == 0){
			STATS_print_packet_stats();
		}
		
		// After max time send stop command ('=') and print driver statistics
		if(counter == (MAX_APP_TIME)){
			STATS_close_app();
			PROCESS_EXIT();
		}

		// Wait for the periodic timer to expire and then restart the timer.
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		etimer_reset(&timer);
	}

	PROCESS_END();
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

/*---------------------------------------------------------------------------*/
void
STATS_close_app(void){

	STATS_print_driver_stats();
	
	// Send '=' cmd to stop the monitor
	STATS_output_command(cmd_stop);

	// Empty buffers
	RF2XX_STATS_RESET();
	STATS_clear_packet_stats();

	// Reset the network
	if(NETSTACK_ROUTING.node_is_root()){
		NETSTACK_ROUTING.leave_network();
	}
}

/*---------------------------------------------------------------------------*/
void
STATS_print_help(void){
	uint8_t addr[8];
  radio_value_t rv;

	rf2xx_driver.get_object(RADIO_PARAM_64BIT_ADDR, &addr, 8);
	printf("Device ID: ");
	for(int j=0; j<8; j++){
		printf("%X",addr[j]);
	}

  printf("\n"); 

  rf2xx_driver.get_value(RADIO_PARAM_CHANNEL, &rv);
  printf("Set on channel %d \n", rv);
	
	printf("----------------------------------------------------------------------------\n");
	printf("\n");
	printf("       DESCRIPTION\n");
	printf("----------------------------------------------------------------------------\n");
	printf("BGN [time-stamp (channel)RSSI] [time-stamp (channel)RSSI] [ ...\n");
	printf("\n");
	printf("Tx [time-stamp] packet-type  dest-addr (chn len sqn | pow) BC or UC \n");
	printf("Rx [time-stamp] packet-type  sour-addr (chn len sqn | rssi lqi) \n");
	printf("\n");
	printf("On the end of file, there is a count of all received and transmited packets. \n");
	printf("----------------------------------------------------------------------------\n");
}