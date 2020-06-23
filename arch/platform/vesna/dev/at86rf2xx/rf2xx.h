#ifndef RF2XX_H_
#define RF2XX_H_

#include "contiki-net.h"

// String for debug purposes
#ifndef AT86RF2XX_BOARD_STRING
#define AT86RF2XX_BOARD_STRING "Unknown"
#endif


// Default log level
#ifndef LOG_CONF_LEVEL_RF2XX
#define LOG_LEVEL_RF2XX     (LOG_LEVEL_WARN)
#else
#define LOG_LEVEL_RF2XX     (LOG_CONF_LEVEL_RF2XX)
#endif

// Collect packet statistics
#ifndef RF2XX_CONF_PACKET_STATS
#define RF2XX_PACKET_STATS     (0)
#else
#define RF2XX_PACKET_STATS     (RF2XX_CONF_PACKET_STATS)
#endif

// Collect statistics on radio driver operations
#ifndef  RF2XX_CONF_DRIVER_STATS
#define RF2XX_DRIVER_STATS      (0)
#else
#define RF2XX_DRIVER_STATS      (RF2XX_CONF_DRIVER_STATS)
#endif

// AT86RF2xx driver for Contiki(-NG)
extern const struct radio_driver rf2xx_driver;

// Radio driver API
int rf2xx_init(void);

int rf2xx_prepare(const void *payload, unsigned short payload_len);
int rf2xx_transmit(unsigned short payload_len);
int rf2xx_send(const void *payload, unsigned short payload_len);
int rf2xx_read(void *buf, unsigned short buf_len);
int rf2xx_cca(void);
int rf2xx_receiving_packet(void);
int rf2xx_pending_packet(void);
int rf2xx_on(void);
int rf2xx_off(void);

// Interrupt routine function
void rf2xx_isr(void);

// Continuous transmission test mode
void rf2xx_CTTM_start(void);
void rf2xx_CTTM_stop(void);

#endif