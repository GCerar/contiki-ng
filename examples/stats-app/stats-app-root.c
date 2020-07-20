/* -----------------------------------------------------------------------------
 * STATS-APP 
 * -----------------------------------------------------------------------------
*/
#include <stdio.h>
#include "dev/serial-line.h"
#include <string.h>
#include <inttypes.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "net/ipv6/uip.h"
#include "net/routing/rpl-classic/rpl-private.h"
#include "net/ipv6/multicast/uip-mcast6.h"

#include "arch/platform/vesna/dev/at86rf2xx/rf2xx.h"
#include "arch/platform/vesna/dev/at86rf2xx/rf2xx_stats.h"

#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"
#include "net/routing/routing.h"


#if !NETSTACK_CONF_WITH_IPV6 || !UIP_CONF_ROUTER || !UIP_IPV6_MULTICAST || !UIP_CONF_IPV6_RPL
#error "This example can not work with the current contiki configuration"
#error "Check the values of: NETSTACK_CONF_WITH_IPV6, UIP_CONF_ROUTER, UIP_CONF_IPV6_RPL"
#endif

/*---------------------------------------------------------------------------*/
#define SECOND 		  (1000)
#define BGN_MEASURE_TIME_MS (10)
#define MAX_APP_TIME  (60 * 90) //90min

#define MAX_PAYLOAD_LEN 120
#define MCAST_SINK_UDP_PORT 3001 /* Host byte order */
#define SEND_INTERVAL CLOCK_SECOND /* clock ticks */
#define ITERATIONS 1000 /* messages */
#define START_DELAY 60 //Start sending messages START_DELAY secs after we start

uint32_t counter = 0;

static struct uip_udp_conn * mcast_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint32_t seq_id = 150;

enum STATS_commands {cmd_start, cmd_stop, app_duration};
/*---------------------------------------------------------------------------*/
void STATS_print_help(void);
void STATS_input_command(char *data);
void STATS_output_command(uint8_t cmd);
void STATS_set_device_as_root(void);
void STATS_close_app(void);

void STATS_prepare_mcast(void);
void STATS_multicast_send(void);
/*---------------------------------------------------------------------------*/
PROCESS(stats_process, "Stats app process");
PROCESS(serial_input_process, "Serial input command");
PROCESS(bgn_process, "Background noise process");

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
		//process_start(&bgn_process, NULL);
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
PROCESS_THREAD(bgn_process, ev, data)
{
	static struct etimer bgn_timer;

	PROCESS_BEGIN();

	etimer_set(&bgn_timer, BGN_MEASURE_TIME_MS);

	while(1){
		STATS_update_background_noise();

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&bgn_timer));
		etimer_reset(&bgn_timer);
	}
	PROCESS_END();
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
	STATS_clear_background_noise();

	// Send app duration to LGTC
	STATS_output_command(app_duration);

	STATS_print_help();

	// Set device as root
	STATS_set_device_as_root();

	// Prepare multicast engine
	printf("Multicast engine: '%s' \n",UIP_MCAST6.name);
	STATS_prepare_mcast();

	etimer_set(&timer, SECOND);

	while(1) {
		counter++;	

		if((counter % 10) == 0){
			STATS_print_packet_stats();
			//STATS_print_background_noise();
		}

		// After one minute, send MC message every second
		if(counter > START_DELAY){
			STATS_multicast_send();
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

	//process_exit(&bgn_process);

	STATS_print_driver_stats();
	
	// Send '=' cmd to stop the monitor
	STATS_output_command(cmd_stop);

	// Empty buffers
	RF2XX_STATS_RESET();
	STATS_clear_background_noise();
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


/*---------------------------------------------------------------------------*/
void
STATS_multicast_send(void)
{
  uint32_t id;

  id = uip_htonl(seq_id);
  memset(buf, 0, MAX_PAYLOAD_LEN);
  memcpy(buf, &id, sizeof(seq_id));

  PRINTF("Send to: ");
  PRINT6ADDR(&mcast_conn->ripaddr);
  PRINTF(" Remote Port %u,", uip_ntohs(mcast_conn->rport));
  PRINTF(" (msg=0x%08"PRIx32")", uip_ntohl(*((uint32_t *)buf)));
  PRINTF(" %lu bytes\n", (unsigned long)sizeof(id));

  seq_id++;

  if(seq_id == 200){
    seq_id = 0;
  }
  uip_udp_packet_send(mcast_conn, buf, sizeof(id));
}

void
STATS_prepare_mcast(void)
{
  uip_ipaddr_t ipaddr;

#if UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_MPL
/*
 * MPL defines a well-known MPL domain, MPL_ALL_FORWARDERS, which
 *  MPL nodes are automatically members of. Send to that domain.
 */
  uip_ip6addr(&ipaddr, 0xFF03,0,0,0,0,0,0,0xFC);
#else
  /*
   * IPHC will use stateless multicast compression for this destination
   * (M=1, DAC=0), with 32 inline bits (1E 89 AB CD)
   */
  uip_ip6addr(&ipaddr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
#endif
  mcast_conn = udp_new(&ipaddr, UIP_HTONS(MCAST_SINK_UDP_PORT), NULL);
}