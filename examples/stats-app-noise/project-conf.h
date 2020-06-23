/* Project configuration */

// All logs to LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_RF2XX                       LOG_LEVEL_DBG
#define TSCH_LOG_CONF_PER_SLOT                     (0)

// Defines for app
#define UART1_CONF_BAUDRATE                         (460800)        //460800
#define WATCHDOG_CONF_ENABLED                       (0)
