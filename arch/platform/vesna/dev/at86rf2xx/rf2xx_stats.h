#ifndef RF2XX_STATS_H_
#define RF2XX_STATS_H_

#include "contiki.h"
#include "sys/log.h"
#include "rf2xx.h"
#include "rf2xx_hal.h"  // for SR_CHANNEL

#include "os/net/mac/framer/frame802154.h"


#define RF2XX_STATS_RINGBUF_SIZE    	(20)
#define RF2XX_STATS_RINGBUF_NOISE_SIZE 	(1001)


// PACKETS STATISTICS
/*---------------------------------------------------------------------------*/

typedef struct {
    struct {
		uint32_t s;
		uint32_t us;
	} ts;				// Timestamp

    frame802154_t frame; // SQN, UC/BC, ADDRESS,... 

    uint16_t 	count;
    uint8_t 	channel;
    uint8_t 	power;
    uint8_t 	broadcast;
} txPacket_t;

typedef struct {
    struct {
		uint32_t s;
		uint32_t us;
	} ts;

    frame802154_t frame;

    uint16_t count;
    uint8_t  channel;
    int8_t   rssi;
    uint8_t  lqi;
} rxPacket_t;

typedef struct {
	txPacket_t items[RF2XX_STATS_RINGBUF_SIZE];
	uint8_t head;
    uint8_t tail;
} packet_ringbuf_t;

void STATS_initBuff(void);
void STATS_rxPush(rxFrame_t *raw);
int  STATS_rxPull(rxPacket_t *item);
void STATS_txPush(txFrame_t *raw);
int  STATS_txPull(txPacket_t *item);

void STATS_parse_rxFrame(rxFrame_t *raw, rxPacket_t *out);
void STATS_parse_txFrame(txFrame_t *raw, txPacket_t *out);

void STATS_print_packet_stats(void);
void STATS_clear_packet_stats(void);


//    BACKGROUND NOISE
/*---------------------------------------------------------------------------*/

typedef struct {
	struct {
		uint32_t s;
		uint32_t us;
	} ts;

	uint8_t channel;
	int8_t rssi; // received signal strength index (dBm)
} bgNoise_t;


typedef struct {
	bgNoise_t items[RF2XX_STATS_RINGBUF_NOISE_SIZE];
	uint16_t head;
    uint16_t tail;
} bgn_ringbuf_t;


void STATS_noisePush(const bgNoise_t *noise);
int  STATS_noisePull(bgNoise_t *noise);

void STATS_update_background_noise(void);

void STATS_print_background_noise(void);
void STATS_clear_background_noise(void);


 
//     DRIVER STATISTICS
/*---------------------------------------------------------------------------*/

enum {
	rxDetected,		// Detected packets
	rxSuccess,		// Successfully received packets
	rxToStack,		// Not used in TSCH
	rxAddrMatch,	// Not used in TSCH?

	rxData,			// Received Data packet
	rxBeacon,		// Received Beacon packet
	rxAck,			// Received Acknowledge

	txCollision,	// Not used in TSCH? - channel access failure
	txNoAck,		// Not used in TSCH - no ACK received
	txSuccess,		// Not used in TSCH - it's same as txCount
	txCount,		// Num of sent packets
	txError,		// Num of errors
	txTry,			// Num of TX tries

	txAck,			// Transmitted Ack
	txBeacon,		// Transmitted Beacon
	txData,			// Transmitted Data
	txReqAck,		// Request ACK (unicast packet)

	RF2XX_STATS_COUNT
};

	void STATS_display_driver_stats_inline(void);
	void STATS_display_driver_stats(void);
	void STATS_print_driver_stats(void);

#if RF2XX_DRIVER_STATS
	extern volatile uint32_t rf2xxStats[RF2XX_STATS_COUNT];
	#define RF2XX_STATS_GET(event)		rf2xxStats[event]
	#define RF2XX_STATS_ADD(event)		rf2xxStats[event]++
	#define RF2XX_STATS_RESET()    		memset(rf2xxStats, 0, sizeof(rf2xxStats[0]) * RF2XX_STATS_COUNT)
#else
	#define RF2XX_STATS_GET(event)		(0)
	#define RF2XX_STATS_ADD(event)
	#define RF2XX_STATS_RESET()
#endif


#endif