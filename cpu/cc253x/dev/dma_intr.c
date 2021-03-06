/**
 * \file
 *         DMA driver ISRs
 * \author
 *         Original: Martti Huttunen <martti@sensinode.com>
 *         Port: Zach Shelby <zach@sensinode.com>
 *
 *         DMA interrupt routines, must be stored in HOME bank
 */

#include <stdio.h>

#include "contiki.h"

#include "dev/dma.h"
#include "cc253x.h"

#if DMA_ON
extern struct process *dma_callback[DMA_CHANNEL_COUNT];
#endif

/*---------------------------------------------------------------------------*/
#ifdef HAVE_RF_DMA
extern void rf_dma_callback_isr(void);
#endif
#ifdef SPI_DMA_RX
extern void spi_rx_dma_callback(void);
#endif
/*---------------------------------------------------------------------------*/
/**
 * DMA interrupt service routine.
 *
 * if callback defined a poll is made to that process
 */
/* Avoid referencing bits, we don't call code which use them */
#if defined(__SDCC_mcs51) || defined(SDCC_mcs51)
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
#endif
#if defined __IAR_SYSTEMS_ICC__
#pragma vector=DMA_VECTOR
__near_func __interrupt void dma_isr(void)
#else
void
dma_isr(void) __interrupt(DMA_VECTOR)
#endif
{
#if DMA_ON
  uint8_t i;
#endif
  EA = 0;
  DMAIF = 0;
#ifdef HAVE_RF_DMA
  if((DMAIRQ & 1) != 0) {
    DMAIRQ = ~1;
    DMAARM = 0x81;
    rf_dma_callback_isr();
  }
#endif
#ifdef SPI_DMA_RX
  if((DMAIRQ & 0x08) != 0) {
    DMAIRQ = ~(1 << 3);
    spi_rx_dma_callback();
  }
#endif
#if DMA_ON
  for(i = 0; i < DMA_CHANNEL_COUNT; i++) {
    if((DMAIRQ & (1 << i)) != 0) {
      DMAIRQ = ~(1 << i);
      if(dma_callback[i] != 0) {
        process_poll(dma_callback[i]);
      }
    }
  }
#endif
  EA = 1;
}
#pragma restore
/*---------------------------------------------------------------------------*/
