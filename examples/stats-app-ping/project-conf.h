/* Project configuration */

// All logs to LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_RF2XX                       LOG_LEVEL_DBG
#define TSCH_LOG_CONF_PER_SLOT                     (0)

// Defines for app
#define UART1_CONF_BAUDRATE                         (460800)        //460800
#define WATCHDOG_CONF_ENABLED                       (1)

#define RF2XX_CONF_PACKET_STATS                     (0)
#define RF2XX_CONF_DRIVER_STATS                     (0)

#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){ 11, 15, 20, 26 }

// Testbed can have max 21 devices
#define NETSTACK_MAX_ROUTE_ENTRIES                  (25)
#define NBR_TABLE_CONF_MAX_NEIGHBORS                (25)