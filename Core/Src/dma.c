#include "dma.h"

#define MEM2PER		0b01
#define BYTE		0b00

void dma1_init(void)
{
	/* DMA clock en */
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);

	CLEAR_BIT(DMA1_Stream6->CR, DMA_SxCR_EN);
	while(READ_BIT(DMA1_Stream6->CR, DMA_SxCR_EN));
	/* DMA Ch set */
	MODIFY_REG(DMA1_Stream6->CR, DMA_SxCR_CHSEL, 4<<DMA_SxCR_CHSEL_Pos);
	/* Data direction */
	MODIFY_REG(DMA1_Stream6->CR, DMA_SxCR_DIR, MEM2PER<<DMA_SxCR_DIR_Pos);
	/* Circular mode */
//	SET_BIT(DMA1_Stream6->CR, DMA_SxCR_CIRC);
	/* Memory inc */
	SET_BIT(DMA1_Stream6->CR, DMA_SxCR_MINC);
	/* Perif inc */
	CLEAR_BIT(DMA1_Stream6->CR, DMA_SxCR_PINC);
	/* Memory data size */
	MODIFY_REG(DMA1_Stream6->CR, DMA_SxCR_MSIZE, BYTE<<DMA_SxCR_MSIZE_Pos);
	/* Perif data size */
	MODIFY_REG(DMA1_Stream6->CR, DMA_SxCR_PSIZE, BYTE<<DMA_SxCR_PSIZE_Pos);
	/* Interrupt en*/
	SET_BIT(DMA1_Stream6->CR, DMA_SxCR_TCIE);
	SET_BIT(DMA1_Stream6->CR, DMA_SxCR_TEIE);

	NVIC_SetPriority(DMA1_Stream6_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

bool dma1_to_periph_start(uint32_t src_addr, uint32_t dst_addr, uint16_t data_size)
{
	if (READ_BIT(DMA1_Stream6->CR, DMA_SxCR_EN))
	{
		/* Previous transaction hasn't finished yet*/
		return false;
	}
	WRITE_REG(DMA1_Stream6->NDTR, data_size<<DMA_SxNDT_Pos);
	WRITE_REG(DMA1_Stream6->M0AR, src_addr<<DMA_SxM0AR_M0A_Pos);
	WRITE_REG(DMA1_Stream6->PAR, dst_addr<<DMA_SxPAR_PA_Pos);
	SET_BIT(DMA1_Stream6->CR, DMA_SxCR_EN);

	return true;
}
