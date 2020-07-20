/* Project configuration */
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// All logs to LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_RF2XX                       LOG_LEVEL_WARN
#define TSCH_LOG_CONF_PER_SLOT                     (0)

// Defines for app
#define UART1_CONF_BAUDRATE                         (460800)        //460800
#define WATCHDOG_CONF_ENABLED                       (0)

#define RF2XX_CONF_PACKET_STATS                     (1)
#define RF2XX_CONF_DRIVER_STATS                     (1)

#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){ 15 }
#define IEEE802154_CONF_DEFAULT_CHANNEL             (15)

// PAN-ID for FFF experiment - to join the same network
#define IEEE802154_CONF_PANID                       (0xBEEF)

// Testbed can have max 21 devices
#define NETSTACK_MAX_ROUTE_ENTRIES                  (25)
#define NBR_TABLE_CONF_MAX_NEIGHBORS                (25)

#endif /* PROJECT_CONF_H_ */