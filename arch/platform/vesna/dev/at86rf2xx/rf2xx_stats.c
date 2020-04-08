#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rf2xx_stats.h"
#include "rf2xx.h"
#include "heapmem.h"
#include "sys/log.h"

#define LOG_MODULE  "STATS"
#define LOG_LEVEL   LOG_LEVEL_INFO

//TODO
//#if RF2XX_STATS

static packet_ringbuf_t rx_ringbuf;
static packet_ringbuf_t tx_ringbuf;
static bgn_ringbuf_t bgn_ringbuf;

void
STATS_initBuff(void)
{
    rx_ringbuf.head = rx_ringbuf.tail;
    tx_ringbuf.head = tx_ringbuf.tail;
    bgn_ringbuf.head = bgn_ringbuf.tail;
}

/**********************************************************************************************************
 * PACKETS STATISTICS
 **********************************************************************************************************/

void
STATS_txPush(txFrame_t *raw)
{
    static uint32_t count = 0;

    // TODO: Ce dam const pred txPacket_t, tko da bo tudi za memcpy const podatek,
    // mi potem meče error: assigment of member 'power' in read-only object - ko ga 
    // dolocam v parse_txFrame
    txPacket_t packet;
    STATS_parse_txFrame(raw, &packet);

    count++;
    packet.count = count;

    // Copy data into buffer
    memcpy(tx_ringbuf.items + tx_ringbuf.head, &packet, sizeof(txPacket_t));

    tx_ringbuf.head = (tx_ringbuf.head + 1) % RF2XX_STATS_RINGBUF_SIZE;

     // If we are filling buffer too fast, drop oldest entry
    if (tx_ringbuf.tail == tx_ringbuf.head){
        tx_ringbuf.tail = (tx_ringbuf.tail + 1) % RF2XX_STATS_RINGBUF_SIZE;
        // printf("Drop oldest --- make larger ring buffer! \n");
    }

}

int
STATS_txPull(txPacket_t *item)
{
    // Pointers at same position means empty buffer
    if (tx_ringbuf.tail == tx_ringbuf.head) {
        return 0;
    }

    // Copy data from buffer
    memcpy(item, tx_ringbuf.items + tx_ringbuf.tail, sizeof(txPacket_t));
    tx_ringbuf.tail = (tx_ringbuf.tail + 1) % RF2XX_STATS_RINGBUF_SIZE;

    return 1;
}


void
STATS_rxPush(rxFrame_t *raw)
{
    static uint32_t count = 0;
    rxPacket_t packet;

    STATS_parse_rxFrame(raw, &packet);

    count++;
    packet.count = count;

    // Copy data into buffer
    memcpy(rx_ringbuf.items + rx_ringbuf.head, &packet, sizeof(rxPacket_t));

    rx_ringbuf.head = (rx_ringbuf.head + 1) % RF2XX_STATS_RINGBUF_SIZE;

    // If we are filling buffer too fast, drop oldest entry
    if (rx_ringbuf.tail == rx_ringbuf.head) {
        rx_ringbuf.tail = (rx_ringbuf.tail + 1) % RF2XX_STATS_RINGBUF_SIZE;
        //printf("Drop oldest ---> make larger ring buffer! \n");
    }
}

int
STATS_rxPull(rxPacket_t *item)
{
    // If empty buffer
    if (rx_ringbuf.tail == rx_ringbuf.head) {
        return 0;
    }

    // Copy data from buffer
    memcpy(item, rx_ringbuf.items + rx_ringbuf.tail, sizeof(rxPacket_t));

    rx_ringbuf.tail = (rx_ringbuf.tail + 1) % RF2XX_STATS_RINGBUF_SIZE;

    return 1;
}


void
STATS_parse_rxFrame(rxFrame_t *raw, rxPacket_t *out)
{
    radio_value_t rv;

    // Record time at the reception
    vsnTime_preciseUptime(&out->ts.s, &out->ts.us);

    // Use contiki's rx parser to extract data from raw frame
    frame802154_parse(raw->content, raw->len, &out->frame);

    switch (out->frame.fcf.frame_type) {
    case FRAME802154_BEACONFRAME:
        RF2XX_STATS_ADD(rxBeacon);
        break;

    case FRAME802154_DATAFRAME:
        RF2XX_STATS_ADD(rxData);
        break;

    case FRAME802154_ACKFRAME:
        RF2XX_STATS_ADD(rxAck);
        break;
    
    default:
        break;
    }

    out->rssi = raw->rssi;
    out->lqi = raw->lqi;

    rf2xx_driver.get_value(RADIO_PARAM_CHANNEL, &rv);
    out->channel = (uint8_t)rv;
}


void
STATS_parse_txFrame(txFrame_t *raw, txPacket_t *out)
{
    radio_value_t rv;

    // record time at the reception
    vsnTime_preciseUptime(&out->ts.s, &out->ts.us);

    // Use contiki parser to extract data from raw frame
    frame802154_parse(raw->content, raw->len, &out->frame);

    switch (out->frame.fcf.frame_type)
    {
        case FRAME802154_BEACONFRAME:
            RF2XX_STATS_ADD(txBeacon);
            break;

        case FRAME802154_DATAFRAME:
            RF2XX_STATS_ADD(txData);
            break;

        case FRAME802154_ACKFRAME:
            RF2XX_STATS_ADD(txAck);
            break;
        
        default:
            break;
    }

    // Check if it is broadcast
    out->broadcast = frame802154_is_broadcast_addr(out->frame.fcf.dest_addr_mode, out->frame.dest_addr);

    // Check if ACK is required
    if(out->frame.fcf.ack_required){
        RF2XX_STATS_ADD(txReqAck);
    }

    rf2xx_driver.get_value(RADIO_PARAM_CHANNEL, &rv);
    out->channel = rv;

    rf2xx_driver.get_value(RADIO_PARAM_TXPOWER, &rv);
    out->power = (uint8_t)rv;
}

void
STATS_print_packet_stats(void){
    rxPacket_t rxPacket;
    txPacket_t txPacket;

    printf("\n");

    while(STATS_txPull(&txPacket)){

        printf("T%3d ",txPacket.count);

        printf("[%3ld:%6ld] ", txPacket.ts.s, txPacket.ts.us);

        switch(txPacket.frame.fcf.frame_type){
        case 0:
            printf("B 0x%2X%2X ", txPacket.frame.dest_addr[6], txPacket.frame.dest_addr[7]);
            break;
        case 1:
            printf("D 0x%2X%2X ", txPacket.frame.dest_addr[6], txPacket.frame.dest_addr[7]);
            break;
        case 2:
            printf("A 0x%2X%2X ", txPacket.frame.dest_addr[6], txPacket.frame.dest_addr[7]);
            break;
        default:
            printf("Undef ");
            break;
        }

        printf("(C%3d L%3d S%3d | P", txPacket.channel, txPacket.frame.payload_len, txPacket.frame.seq);
        switch(txPacket.power){
            case 0x0:
                printf("3.0"); break;
            case 0x1:
                printf("2.8"); break;
            case 0x2:
                printf("2.3"); break;
            case 0x3:
                printf("1.8"); break;
            case 0x4:
                printf("1.3"); break;
            case 0x5:
                printf("0.7"); break;
            case 0x6:
                printf("0.0"); break;
            case 0x7:
                printf("-1"); break;
            case 0x8:
                printf("-2"); break;
            case 0x9:
                printf("-3"); break;
            case 0xa:
                printf("-4"); break;
            case 0xb:
                printf("-5"); break;
            case 0xc:
                printf("-7"); break;
            case 0xd:
                printf("-9"); break;
            case 0xe:
                printf("-12"); break;
            case 0xf:
                printf("-17"); break;
        }
        if(txPacket.broadcast) printf(") B\n");
        else printf(") U\n");
        
    }

    while(STATS_rxPull(&rxPacket)){

        printf("R%3d ",rxPacket.count);
        printf("[%3ld:%6ld] ", rxPacket.ts.s, rxPacket.ts.us);

        switch(rxPacket.frame.fcf.frame_type){
        case 0:
            printf("B 0x%2X%2X ", rxPacket.frame.src_addr[6], rxPacket.frame.src_addr[7]);
            break;
        case 1:
            printf("D 0x%2X%2X ", rxPacket.frame.src_addr[6], rxPacket.frame.src_addr[7]);
            break;
        case 2:
            printf("A        ");    //Ack has no source address
            break;
        default:
            printf("Undef ");
            break;
        }
        printf("(C%3d L%3d S%3d | R%+3d Q%3d)\n",rxPacket.channel, rxPacket.frame.payload_len, rxPacket.frame.seq, rxPacket.rssi, rxPacket.lqi);
    }
}


void 
STATS_clear_packet_stats(void)
{  
    rx_ringbuf.head = rx_ringbuf.tail;
    tx_ringbuf.head = tx_ringbuf.tail;
}

/**********************************************************************************************************
 * BACKGROUND NOISE
 **********************************************************************************************************/
//static bgn_ringbuf_t bgn_ringbuf;

void
STATS_noisePush(const bgNoise_t *noise)
{
    // Critical section
    memcpy(bgn_ringbuf.items + bgn_ringbuf.head, noise, sizeof(bgNoise_t));

    bgn_ringbuf.head = (bgn_ringbuf.head + 1) % RF2XX_STATS_RINGBUF_NOISE_SIZE;

    // If we are filling buffer too fast, drop oldest entry
    if (bgn_ringbuf.head == bgn_ringbuf.tail){
        bgn_ringbuf.tail = (bgn_ringbuf.tail + 1) % RF2XX_STATS_RINGBUF_NOISE_SIZE;
        printf("Make larger noise buffer! \n");
    }
}


int
STATS_noisePull(bgNoise_t *noise)
{
    // Buffer is empty
    if (bgn_ringbuf.tail == bgn_ringbuf.head) {
        return 0;
    }

    // TODO: Critical section
    memcpy(noise, bgn_ringbuf.items + bgn_ringbuf.tail, sizeof(bgNoise_t));
    bgn_ringbuf.tail = (bgn_ringbuf.tail + 1) % RF2XX_STATS_RINGBUF_NOISE_SIZE;

    return 1;
}


void
STATS_update_background_noise(void)
{
    // rf2xx_driver.on();
    bgNoise_t bgn;
    radio_value_t rv;

    vsnTime_preciseUptime(&bgn.ts.s, &bgn.ts.us);

    // Read channel through driver (so it can be cached for faster access)
    rf2xx_driver.get_value(RADIO_PARAM_CHANNEL, &rv);
    bgn.channel = (uint8_t)rv;
    
    // Access RSS(I) value through driver to avoid duplicate calculation
    rf2xx_driver.get_value(RADIO_PARAM_RSSI, &rv);
    bgn.rssi = (int8_t)rv;

    STATS_noisePush((const bgNoise_t *)&bgn);
}


void
STATS_print_background_noise(void)
{
    bgNoise_t bgn;

    printf("BGN ");
    while(STATS_noisePull(&bgn)){
        printf("[%ld:%ld (CH%d)%+d] ", bgn.ts.s, bgn.ts.us, bgn.channel, bgn.rssi);
    }
    printf("\n");
}

void
STATS_clear_background_noise(void)
{
    bgn_ringbuf.head = bgn_ringbuf.tail;
}




/**********************************************************************************************************
 * DRIVER STATISTICS
 **********************************************************************************************************/

extern const struct radio_driver rf2xx_driver;

/** @brief Costum function for stats-app.c. --> HUMAN READABLE
 *         Print all driver statistics.
 * 
 *  ------- TX STATISTICS -------
    Success: 36 | Error: 0
    * Beac: 4
    * Data: 20
    * Ackn: 12
    ------- RX STATISTICS -------
    Success: 39 | Detected: 39
    * Beac: 6
    * Data: 16
    * Ackn: 17 -> Requested 17
 */
void
STATS_display_driver_stats(void){
    LOG_INFO("------- TX STATISTICS -------\n");
    LOG_INFO("Success: %ld | Error: %ld\n",RF2XX_STATS_GET(txCount),RF2XX_STATS_GET(txError));  //txCount == txSuccess in TSCH
    LOG_INFO(" * Beac: %ld\n", RF2XX_STATS_GET(txBeacon));
    LOG_INFO(" * Data: %ld\n", RF2XX_STATS_GET(txData));
    LOG_INFO(" * Ackn: %ld\n", RF2XX_STATS_GET(txAck));
    
    LOG_INFO("------- RX STATISTICS -------\n");
    LOG_INFO("Success: %ld | Detected: %ld\n", RF2XX_STATS_GET(rxSuccess), RF2XX_STATS_GET(rxDetected));
    LOG_INFO(" * Beac: %ld\n",RF2XX_STATS_GET(rxBeacon));
    LOG_INFO(" * Data: %ld\n",RF2XX_STATS_GET(rxData));
    LOG_INFO(" * Ackn: %ld -> Requested %ld\n", RF2XX_STATS_GET(rxAck),RF2XX_STATS_GET(txReqAck));
    LOG_INFO_("\n");
}

/** @brief Costum function for stats-app.c.--> FOR SERIAL MONITOR
 *         Print all driver statistics.
 * 
 *  TX suc36 err0  cnt: B4 D20 A12
    RX suc39 det39 cnt: B6 D16 A17 -> req(17)
 */
void
STATS_print_driver_stats(void){
    printf("\n");
    printf("TX suc%ld err%ld  cnt: B%ld D%ld A%ld\n",
            RF2XX_STATS_GET(txCount),
            RF2XX_STATS_GET(txError),
            RF2XX_STATS_GET(txBeacon),
            RF2XX_STATS_GET(txData),
            RF2XX_STATS_GET(txAck)
    );
    printf("RX suc%ld det%ld cnt: B%ld D%ld A%ld -> req(%ld)\n",
            RF2XX_STATS_GET(rxSuccess), 
            RF2XX_STATS_GET(rxDetected),
            RF2XX_STATS_GET(rxBeacon),
            RF2XX_STATS_GET(rxData),
            RF2XX_STATS_GET(rxAck),
            RF2XX_STATS_GET(txReqAck)
    );
}

/** @brief Costum function for stats-app.c. --> FOR DEBUGGING PURPOSE
 *         Print small in-line staistics.
 * 
 *  CH 26 [currRSSI -94, lastRSSI -82, LQI 255]  R[38(38)]  T[35(35)]
    CH 20 [currRSSI -94, lastRSSI -82, LQI 255]  R[39(39)]  T[36(36)]
    CH 15 [currRSSI -94, lastRSSI -82, LQI 255]  R[42(42)]  T[37(37)]
 */
void
STATS_display_driver_stats_inline(void){
    radio_value_t channel;
    radio_value_t lastRssi;
    radio_value_t currRssi;
    radio_value_t lqi;

    rf2xx_driver.get_value(RADIO_PARAM_CHANNEL, &channel);
    rf2xx_driver.get_value(RADIO_PARAM_RSSI, &currRssi);
    rf2xx_driver.get_value(RADIO_PARAM_LAST_RSSI, &lastRssi);
    rf2xx_driver.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi);

    LOG_INFO("CH %2d [currRSSI %d, lastRSSI %d, LQI %d]  R[%ld(%ld)]  T[%ld(%ld)]\n",
        channel,                        // Current channel
        currRssi,                       // Current RSSI value
        lastRssi,                       // Last RSSI measured
        lqi,                            // Last LQI measured

        RF2XX_STATS_GET(rxDetected),    // Num of detected packets
        RF2XX_STATS_GET(rxSuccess),     // Successfully received packets

        RF2XX_STATS_GET(txTry),         // Num of transmissions
        RF2XX_STATS_GET(txSuccess)      // Successfull transmissions
    );
}

//#endif