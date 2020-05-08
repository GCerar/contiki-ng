/* Project configuration */

// All logs to LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_NONE   //Change this for more info
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_RF2XX                       LOG_LEVEL_NONE
#define TSCH_LOG_CONF_PER_SLOT                     (0)

// Defines for app
#define RF2XX_CONF_STATS                           (1)

#define STATS_DEBUGG                               (0)

#define STATS_PING_NBR                             (0)

#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_4_4 //(uint8_t[]){ 15, 20, 26 }
//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_16_16
