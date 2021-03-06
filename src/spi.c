#include "spi.h"

/********* Spi_init *******
*  Initializes the SPI peripheral for simplex serial transmission.
*  8 bits per word
*   Inputs: none
*  Outputs: none
*/
void Spi_init(void)
{
	spi_reset(SPI1);

	//spi_i2s_mode_spi_mode(SPI1);
	
	// Baudrate ( 48000000 / 2 ) 24 MHz
	spi_init_master( SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_2, 
		SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, 
		SPI_CR1_MSBFIRST );

	spi_set_data_size( SPI1, SPI_CR2_DS_9BIT );

	//spi_set_baudrate_prescaler( SPI1, SPI_CR1_BR_FPCLK_DIV_64 );

	//spi_enable_ss_output(SPI1);
	Spi_enableNssPulse(SPI1);
	//spi_enable_software_slave_management(SPI1);
	//spi_enable_tx_queue_empty_interrupt(SPI1);
	spi_set_bidirectional_transmit_only_mode(SPI1);

	//spi_enable(SPI1);
	

}

/********* Spi_fifoTxEvent *******
*  Periodic event that manages the SPI transmission queue.
*  Executed from the event scheduler.
*   Inputs: Queue_t pointer, signal flag
*  Outputs: none
*/
void Spi_fifoTxEvent( Queue_t *queue, int *flagPt )
{
	int buf[1], len;

	// attempt to read an element from the queue.
	if ( ( len = Queue_get( queue, &buf, 1 ) ) )
	{
		// Wait until transaction is complete
		Sched_flagWait(flagPt);
		
		// Send data to the queue transfer handler function
		queue->handler_function( &buf, len );

		gpio_toggle(GPIOC, GPIO0);

	}

	//gpio_toggle(GPIOB, GPIO3);
}

/********* Spi_dmaTxHandler *******
*  Copies data from a memory address to the SPI peripheral DMA transmission 
*  channel.
*   Inputs: pointer to a contiguous block of data, number of elements to copy
*  Outputs: none
*/
void Spi_dmaTxHandler( volatile void* data, int length )
{
	dma_disable_channel( DMA1, DMA_CHANNEL3 );

	dma_set_memory_address( DMA1, DMA_CHANNEL3, (uint32_t) data );
	dma_set_number_of_data( DMA1, DMA_CHANNEL3, length );

	dma_enable_transfer_complete_interrupt( DMA1, DMA_CHANNEL3 );
	dma_enable_channel( DMA1, DMA_CHANNEL3 );
	
	spi_enable_tx_dma(SPI1);

	spi_enable(SPI1);
	
}

/********* Spi_send *******
*  Adds arbitrary number of elements to the UART transmission buffer.
*   Inputs: pointer to a contiguous block of data, number of elements to copy
*  Outputs: none
*/
void Spi_send( volatile void* data, int length )
{
	Queue_put( &Q_fifo_u8_spi, data, length );

}

/********* Spi_enableNssPulse *******
*  Enables SPI to generate an NSS pulse between two consecutive words while
*  performing consecutive transfers. The NSS is held high when transfers are
*  complete. Applies only to master mode, and has no effect when FRF or
*  CPHA control registers are 1.
*   Inputs: SPI peripheral address identifier
*  Outputs: none
*/
void Spi_enableNssPulse( int spi )
{
	SPI_CR2(spi) |= SPI_CR2_NSSP;
}