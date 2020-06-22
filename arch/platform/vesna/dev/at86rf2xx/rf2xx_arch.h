#ifndef RF2XX_ARCH_H_
#define RF2XX_ARCH_H_

#include <stdint.h>

#define RF2XX_MAX_FRAME_SIZE	(127)
#define RF2XX_MIN_FRAME_SIZE	(3)
#define RF2XX_CRC_SIZE			(2)
#define RF2XX_LQI_SIZE			(1)
#define RF2XX_MAX_PAYLOAD_SIZE	(RF2XX_MAX_FRAME_SIZE - RF2XX_CRC_SIZE)


// The delay between radio Tx request and SFD sent, in rtimer ticks
#define RF2XX_DELAY_BEFORE_TX		((unsigned)US_TO_RTIMERTICKS(290))
// Possible state transitions:
//      -> FORCE_TRX_OFF                    - 1us
//      TRX_OFF -> PLL_ON                   - 110us
//      RX_ON   -> PLL_ON                   - 1us
// Time before start sending
//      PLL_ON  -> BUSY_TX                  - 16us
// Time to send PREAMBLE + SFD (p.39)       
//      (4B + 1B) * 32 us/B                 - 160us 
//                                          = 286us

// The delay between radio Rx request and start listening, in rtimer ticks
#define RF2XX_DELAY_BEFORE_RX		((unsigned)US_TO_RTIMERTICKS(150))
// Possible state transitions:
//       -> FORCE_TRX_OFF                   - 1us
//       TRX_OFF -> RX_ON                   - 110us 
//       PLL_ON -> RX_ON                    - 1us
// Time until PLL_LOCK should occur         - 32us
//                                          = 142us

// The delay between the end of SFD reception and the radio returning 1 to receiving_packet()
#define RF2XX_DELAY_BEFORE_DETECT	((unsigned)US_TO_RTIMERTICKS(40))
// Time after SFD reception (p.39)
//       PHR reception                      - 32us
//       Interrupt latency                  - 9us
//                                          = 41us

// The number of header and footer bytes of overhead at the PHY layer after SFD (1 length + 2 CRC)
#define RF2XX_PHY_OVERHEAD			(3) 

// The air time for one byte in microsecond: 1 / (250kbps/8) == 32 us/byte
#define RF2XX_BYTE_AIR_TIME			(32) 

// The drift compared to "true" 10ms slots (see rtimer-arch.h)
#define RF2XX_BASE_DRIFT_PPM        (RTIMER_ARCH_DRIFT_PPM)


extern const uint16_t tsch_timeslot_timing_rf2xx_10000us_250kbps[];

// TSCH timeslot timing (default is: 10ms tsch_timeslot_timing_us_10000)
#define RF2XX_CONF_DEFAULT_TIMESLOT_TIMING	(tsch_timeslot_timing_rf2xx_10000us_250kbps)


// If driver is built for Contiki's 6Tisch implementation
#if MAC_CONF_WITH_TSCH
#define RF2XX_CONF_AACK         (0)
#define RF2XX_CONF_ARET         (0)
//#define RF2XX_CONF_HW_CCA       (0)
#define RF2XX_CONF_POLLING_MODE (1)
#endif


// Enable radio's auto acknowledge capabilities (extended mode)
#ifndef RF2XX_CONF_AACK
#define RF2XX_AACK   (0)
#else
#define RF2XX_AACK   (RF2XX_CONF_AACK)
#endif

/* CSMA acknowledge configuration  */
#define RF2XX_SEND_SOFT_ACK (!RF2XX_AACK)

// Enable radio's auto retransmission capabilities (extended mode)
#ifndef RF2XX_CONF_ARET
#define RF2XX_ARET  (0)
#else
#define RF2XX_ARET  (RF2XX_CONF_ARET)
#endif

// Enables radio's automatic CCA before sending
#define RF2XX_HW_CCA   (RF2XX_ARET)

// Number of CSMA retries 0-5, 6 = reserved, 7 = immediately without CSMA/CA
#ifndef RF2XX_CONF_CSMA_RETRIES
#define RF2XX_CSMA_RETRIES  (5)
#else
#if RF2XX_CONF_CSMA_RETRIES < 0 || RF2XX_CONF_CSMA_RETRIES > 5
#error "Invalid RF2XX_CONF_CSMA_RETRIES"
#endif
#define RF2XX_CSMA_RETRIES  (RF2XX_CONF_CSMA_RETRIES)
#endif

// Number of frame retries, if no ACK, 0-15 (TX_ARET-only)
#ifndef RF2XX_CONF_FRAME_RETRIES
#define RF2XX_FRAME_RETRIES     (15)
#else
#define RF2XX_FRAME_RETRIES     (RF2XX_CONF_FRAME_RETRIES)
#endif

// Enable offloading checksum calculation to the radio chip
#ifndef RF2XX_CONF_CHECKSUM
#define RF2XX_CHECKSUM  (1)
#else
#define RF2XX_CHECKSUM  (RF2XX_CONF_CHECKSUM)
#endif

// Skip radio's address filter (AACK only)
#ifndef RF2XX_CONF_PROMISCOUS_MODE
#define RF2XX_PROMISCOUS_MODE   (0)
#else
#define RF2XX_PROMISCOUS_MODE   (RF2XX_CONF_PROMISCOUS_MODE)
#endif

#ifndef RF2XX_CONF_POLLING_MODE
#define RF2XX_POLLING_MODE   (0)
#else
#define RF2XX_POLLING_MODE   (RF2XX_CONF_POLLING_MODE)
#endif

#endif