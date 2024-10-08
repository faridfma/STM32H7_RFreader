/*
 * RFIDreader.c
 *
 *  Created on: Sep 20, 2024
 *      Author: fmabrouk
 */


#include "main.h"
#include "string.h"
#include "stdbool.h"
#include <stdio.h>
#include "RFIDreader.h"




// E X T E R N S ///////////////////////////
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern volatile uint16_t RxDataSize;

/* Buffer used for reception */
//uint8_t aRxBuffer[RxBuf_SIZE]={0};

uint8_t __attribute__((section(".RxRF_SRAMSection"))) aRxBuffer[RxBuf_SIZE] = {0};


/* Buffer used for transmission */
const uint8_t RFIDreaderInfoCmd[18] ={0x80,0x01,0x00,0x00,0x00,0x00,0x53,0x58,0x00,0x12,0x00,0x00,0x00,0x08,0x00,0x01,0x00,0x9E};

const uint8_t SetPowerTo25dBm[28]={0x80,0x01,0x00,0x00,0x00,0x00,0x53,0x58,0x00,0x1C,0x00,0x00,
                           0x00,0x08,0x00,0x01,0x00,0x64,0x00,0x00,0x00,0x0A,0x00,0x96,0x00,0x00,0x01,0x3C};

const uint8_t SetProtocolComand[28]={0x80,0x01,0x00,0x00,0x00,0x00,0x53,0x58,0x00,0x1C,0x00,0x00,
		                            0x00,0x08,0x00,0x01,0x00,0x74,0x00,0x00,0x00,0x0A,0x00,0x54,0x00,0x00,0x00,0x03};


const uint8_t InventoryTagCommand[33]={0x80,0x01,0x00,0x00,0x00,0x00,0x53,0x58,0x00,0x21,0x00,0x00,
		                             0x00,0x08,0x00,0x01,0x00,0x13,0x00,0x00,0x00,0x0F,0x00,0xFB,0x53,
									 0x6F,0x75,0x72,0x63,0x65,0x5F,0x30,0x00};

const uint8_t ReadRFreaderTxPowerCmd[18] = {0x80,0x01,0x00,0x00,0x00,0x00,0x53,0x58,0x00,0x12,0x00,0x00,0x00,0x08,0x00,0x01,0x00,0x73};



// GLOBAL VARIABLES
volatile bool DataReceivedFlag = false;
volatile bool RxTagsDataCompleted = false;
volatile bool InventoryTagRequested = false;






void SetUpRFIDreader(void)
{
	HAL_StatusTypeDef status;
	uint16_t RxDataSize = 0;
	uint8_t RFReaderInfoBuff[256] = {0};


	printf("Setting Up the RFID reader\n\r");

//	//request RFID Module info
//	status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)aRxBuffer, RxBuf_SIZE);
//	if(status!= HAL_OK)
//	{
//		Error_Handler();
//	}
//	 __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

	if(HAL_UART_Transmit_DMA(&huart1, (uint8_t*)RFIDreaderInfoCmd,sizeof(RFIDreaderInfoCmd))!= HAL_OK)
	{
		Error_Handler();
	}

	while (DataReceivedFlag == false) {};

	//extract RF reader info
//	RxDataSize = (aRxBuffer[20] << 8) | (aRxBuffer[21]);
//	RxDataSize = RxDataSize - 6;
//
//	memcpy(RFReaderInfoBuff, &aRxBuffer[24], RxDataSize);
//	printf("RF Reader Detected! \n\r");
//	printf("reader part number and serial number are: %s \n\n\r", RFReaderInfoBuff);
//
//	DataReceivedFlag = false;
//	memset(aRxBuffer, 0, sizeof(aRxBuffer));





	//set output power to 25 dBm
//	status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)aRxBuffer, RxBuf_SIZE);
//	if(status!= HAL_OK)
//	{
//		Error_Handler();
//	}
//	 __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

	if(HAL_UART_Transmit_DMA(&huart1, (uint8_t*)SetPowerTo25dBm, sizeof(SetPowerTo25dBm))!= HAL_OK)
	{
		Error_Handler();
	}

	while (DataReceivedFlag == false) {};

//	if ((aRxBuffer[24] == 0x00) && (aRxBuffer[25] == 0x00) && (aRxBuffer[9] == 0x1A))
//    {
//	    printf("Error, Tx power is set to 25dBm \n\r");
//	}
//	else
//	{
//		printf("Error, Tx power is not set to 25dBm \n\r");
//	}
//
//	DataReceivedFlag = false;
//	memset(aRxBuffer, 0, sizeof(aRxBuffer));


	//Read TX power in dBm
//	status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)aRxBuffer, RxBuf_SIZE);
//	if(status!= HAL_OK)
//	{
//		Error_Handler();
//	}
//	 __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

	if(HAL_UART_Transmit_DMA(&huart1, (uint8_t*)ReadRFreaderTxPowerCmd, sizeof(ReadRFreaderTxPowerCmd))!= HAL_OK)
	{
		Error_Handler();
	}

	while (DataReceivedFlag == false) {};

//	if ((aRxBuffer[26]!=0x01) && (aRxBuffer[27] != 0x3C))
//	{
//		printf("Error, Tx power is not set to 25dBm \n\r");
//	}
//	else
//	{
//	    printf("Tx power is set to 25dBm \n\r");
//	}
//
//	DataReceivedFlag = false;
//	memset(aRxBuffer, 0, sizeof(aRxBuffer));

	//SetProtocolComand
//	status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)aRxBuffer, RxBuf_SIZE);
//	if(status!= HAL_OK)
//	{
//		Error_Handler();
//	}
//	 __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

	if(HAL_UART_Transmit_DMA(&huart1, (uint8_t*)SetProtocolComand, sizeof(SetProtocolComand))!= HAL_OK)
	{
		Error_Handler();
	}

	while (DataReceivedFlag == false) {};

//	if ((aRxBuffer[24] == 0x00) && (aRxBuffer[25] == 0x00) && (aRxBuffer[9] == 0x1A))
//	{
//		printf("Protocol is set successfully \n\r");
//	}
//	else
//	{
//		printf("Protocol is not set successfully \n\r");

//	}


	DataReceivedFlag = false;
	memset(aRxBuffer, 0, sizeof(aRxBuffer));
	DataReceivedFlag = false;

	HAL_Delay(1000);
}


void ProcessReceivedData(uint16_t Size)
{
	unsigned char output[28]={0};
	const char hexChars[] = "0123456789ABCDEF";
	uint16_t i, j = 0;
	uint16_t TagIdIndex = 80;

	static uint16_t oldPos = 0;
	static uint16_t newPos = 0;
	static uint16_t ReceivedDataSize = 0;
	static uint8_t MainBuf[8192] = {0};

	static uint8_t TagsCounter = 0;


	//confirm data received is for tag inventory
	if(aRxBuffer[17] == 0x13)
	{
		InventoryTagRequested = true;

		//get the size of data packet
		ReceivedDataSize = aRxBuffer[9] | (aRxBuffer[8] << 8);
	}

	//keep copying data till buffer is full
	if(InventoryTagRequested == true)
	{
	   oldPos = newPos;  // Update the last position before copying new data

	   //copy received data till buffer size is equal to ReceivedDataSize -1
	   memcpy ((uint8_t *)MainBuf+oldPos, aRxBuffer, Size);  // copy data in that remaining space

	   newPos = oldPos + Size;  // Update the last position before copying new data
	}

	if ((ReceivedDataSize == newPos) &&(InventoryTagRequested == true))
	{

	   do {
			 for (i = TagIdIndex; i < TagIdIndex + 12; i++)
			 {
				 output[j++] = hexChars[MainBuf[i] >> 4];
				 output[j++] = hexChars[MainBuf[i] & 0x0F];  // Low nibble
			 }

			 output[j] = '\0'; // Null-terminate the string
			 j++;
			 output[j] = '\n';
			 j++;
			 output[j] = '\r';

			 if(HAL_UART_Transmit(&huart5, (uint8_t*)output, 27, 1000)!= HAL_OK)
			 {
			 	Error_Handler();
			 }

	         j =0;
	         i =0;
	         memset(output, 0, sizeof(output));

	         TagIdIndex = TagIdIndex + 74;

	         TagsCounter++;

	     } while (TagIdIndex < ReceivedDataSize);

	   printf("Number of tags responded with ID is: %d \n\n\r", TagsCounter);

	   TagsCounter = 0;
	   InventoryTagRequested = false;

	   oldPos = 0;
	   newPos = 0;
	   ReceivedDataSize = 0;
	   memset(MainBuf, 0, sizeof(MainBuf));
	   RxTagsDataCompleted = true;
   }

}


void ScanForTags(void)
{

	DataReceivedFlag == false;
	uint16_t Size;

	memset(aRxBuffer,0, RxBuf_SIZE);

	if(HAL_UART_Transmit_DMA(&huart1, (uint8_t*)InventoryTagCommand,33)!= HAL_OK)
	 {
		Error_Handler();
	 }

	while (DataReceivedFlag == false) {};

//	Size = ReadRxDataSize();
//
//	ProcessReceivedData(Size);

//	 while (RxTagsDataCompleted == false) {};
}


uint16_t ReadRxDataSize(void)
{
	return(RxDataSize);
}



//
//bool RFIDreaderSendCmd(volatile uint8_t *aRxBuffer, const uint8_t *RFIDreaderTxCommand, uint8_t Size)
//{
//	bool status = false;
//	uint8_t RFIDreaderTxBuffer[256] = {0};
//	volatile HAL_StatusTypeDef DMAstatus;
//
//	snprintf(RFIDreaderTxBuffer, sizeof(RFIDreaderTxCommand), RFIDreaderTxCommand);
//
//    if (RFIDreaderTxBuffer == NULL || RFIDreaderTxCommand == NULL)
//    {
//	   // Handle null pointer error
//	   return false;
//    }
//
//    status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)aRxBuffer, RxBuf_SIZE);
//	if(status!= HAL_OK)
//	{
//		if(huart1.ReceptionType != HAL_UART_RECEPTION_TOIDLE)
//		{
//			printf("huart4.ReceptionType is set to 0; it will be set to 1 \n\r");
//
//			huart1.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
//		    __HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_IDLEF);
//		     ATOMIC_SET_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);
//		}
//		else
//		{
//		  printf("DMA Error1 \n\r");
//		}
//	}
//
//	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
//
//	if(HAL_UART_Transmit_DMA(&huart1, (uint8_t*)RFIDreaderTxBuffer, sizeof(RFIDreaderTxBuffer))!= HAL_OK)
//	{
//		  printf("DMA Error2 \n\r");
//		  return false;
//	}
//
////	while(LoRaDataReceivedFlag == false) {};
////
////	status = ProcessRxLoraDataForConsoleDisplay(LoRaRxBuffer, LoRaRxBuf_SIZE);
////
////	LoRaDataReceivedFlag = false;
//
//	//ProcessReceivedData(Size);
//
//	while(DataReceivedFlag == false) {};
//
//	//ProcessReceivedData(Size);
//
//
//	DataReceivedFlag = false;
//
//	return true;
//}
