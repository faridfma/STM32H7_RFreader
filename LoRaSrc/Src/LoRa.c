#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stdbool.h"
#include "LoRa.h"



extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_uart4_tx;

extern volatile bool LoRaDataReceivedFlag;
extern volatile bool LoRaDataTransmitedFlag;


//volatile uint8_t LoRaTxBuffer[256] = {0};
//volatile uint8_t LoRaRxBuffer[256] = {0};


volatile __attribute__((section(".LoRa_SRAMSection"))) LoRaRxBuffer[LoRaRxBuf_SIZE] = {0};

volatile __attribute__((section(".LoRa_SRAMSection"))) LoRaTxBuffer[LoRaRxBuf_SIZE]= {0};

static bool LoRaMsgSent = false;



const uint8_t LoRaATcommand[] = "AT\r";
const uint8_t LoRaGetBaudRateATcommand[] =  "AT+UART?\r";
const uint8_t LoRaGetVersionATcommand[] =  "AT+VER?\r";
const uint8_t LoRaGetDevATcommand[] =  "AT+DEV?\r";
const uint8_t LoRaGetBandATcommand[] =  "AT+BAND?\r";
const uint8_t LoRaGetClassATcommand[] =  "AT+CLASS?\r";
const uint8_t LoRaSetUS915BandATcommand[] =  "AT+BAND=8\r";
const uint8_t LoRaGetDevAddrATcommand[] =  "AT+DEVADDR?\r";
const uint8_t LoRaGetDevEUIATcommand[] =  "AT+DEVEUI?\r";
const uint8_t LoRaGetAppEUIATcommand[] =  "AT+APPEUI?\r";
const uint8_t LoRaGetNetworkSessionKeyATcommand[] =  "AT+NWKSKEY?\r";
const uint8_t LoRaGetAppSessionKeyATcommand[] =  "AT+APPSSKEY?\r";
const uint8_t LoRaJoinATcommand[] =  "AT+JOIN\r";
const uint8_t LoRaLinkCheckATcommand[] =  "AT+LNCHECK\r";
const uint8_t LoRaGetRFParametersATcommand[] =  "AT+RFPARAM?\r";
const uint8_t LoRaGetRFPowerATcommand[] =  "AT+RFPOWER?\r";
const uint8_t LoRaGetDataRateATcommand[] =  "AT+DR?\r";
const uint8_t LoRaGetRxChannelFreqATcommand[] =  "AT+RX2?\r";
const uint8_t LoRaGetPortATcommand[] =  "AT+PORT?\r";
const uint8_t LoRaSendHelloMsgATcommand[] =  "AT+UTX 5\rhello\r";
const uint8_t LoRaGetRssiATcommand[] =  "AT+RFQ?\r";



bool TestLoRaModuleComms(volatile uint8_t *LoRaRxBuffer, const uint8_t *LoRaTxCommand, uint8_t Size)
{
	bool status = false;
	volatile HAL_StatusTypeDef DMAstatus;

	snprintf(LoRaTxBuffer, sizeof(LoRaTxBuffer), LoRaTxCommand);

    if (LoRaRxBuffer == NULL || LoRaTxCommand == NULL)
    {
	   // Handle null pointer error
	   return false;
    }

    DMAstatus = HAL_UARTEx_ReceiveToIdle_DMA(&huart4, (uint8_t *)LoRaRxBuffer, 256);
	if(DMAstatus!= HAL_OK)
	{
		if(huart4.ReceptionType != HAL_UART_RECEPTION_TOIDLE)
		{
			printf("huart4.ReceptionType is set to 0; it will be set to 1 \n\r");

			huart4.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
		    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_IDLEF);
		     ATOMIC_SET_BIT(huart4.Instance->CR1, USART_CR1_IDLEIE);
		}
		else
		{
		  printf("DMA Error1 \n\r");
		}
	}

	__HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);

	if(HAL_UART_Transmit_DMA(&huart4, (uint8_t*)LoRaTxBuffer, Size-1)!= HAL_OK)
	{
		  printf("DMA Error2 \n\r");
		  return false;
	}

	while(LoRaDataReceivedFlag == false) {};

	status = ProcessRxLoraDataForConsoleDisplay(LoRaRxBuffer, LoRaRxBuf_SIZE);

	LoRaDataReceivedFlag = false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// public void ProcessReceivedDataForConsoleDisplay(uint16_t Size)
///
/// @brief Function Description : this function process received data when an inventory cycle is requested. it called from DMA receive call back.
///
/// @param  Size : size of data received from DMA receive
/// @retval none
///
bool ProcessRxLoraDataForConsoleDisplay(uint8_t *LoRaRxBuffer, uint16_t Size)
{
	uint8_t OkResponseStr[] = "\r\n\++OK\r\n";
	uint8_t  buffer[20] = {0};

	printf("%s \n\r", LoRaRxBuffer);

	sprintf(buffer, "%s", (uint8_t *)LoRaRxBuffer);


	//if(
	memset(LoRaRxBuffer, 0, LoRaRxBuf_SIZE);

	int result = strncmp((char *)LoRaRxBuffer, (char *)OkResponseStr, Size);

	if((LoRaMsgSent == true) && (result == 0))
	{
		printf("Message sent successfully \n");
	}

	memset(LoRaRxBuffer, 0, LoRaRxBuf_SIZE);

	if (result == 0)
	{
	   return true;
	}
	else
	{
	   return false;
	}
}



bool StartRSSIToneTest(uint8_t *LoRaRxBuffer)
{
	uint8_t LoRaTxBuffer[] = "AT+TRSSI\n";;
	HAL_StatusTypeDef DMAstatus;

    DMAstatus = HAL_UARTEx_ReceiveToIdle_DMA(&huart4, (uint8_t *)LoRaRxBuffer, LoRaRxBuf_SIZE);
	if (DMAstatus!= HAL_OK)
	{
		printf("DMA Error1 \n\r");
		return false;
	}

	__HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);

	if(HAL_UART_Transmit_DMA(&huart4, (uint8_t*)LoRaTxBuffer, sizeof(LoRaTxBuffer))!= HAL_OK)
	{
		  printf("DMA Error2 \n\r");
		  return false;
	}

	while(LoRaDataReceivedFlag == false) {};

	LoRaDataReceivedFlag = false;

	return true;



}
bool SendLoRaDataPacket(uint8_t *LoRaMessage, uint8_t *LoRaRxBuffer, uint8_t Size)
{

	uint8_t  buffer[20] = {0};

	uint8_t *LoRaRxBufferw = "AT+UTX 5\rhello";


	sprintf(buffer, "%s", (uint8_t *)LoRaRxBufferw);


	uint8_t LoRaTxBuffer[50] = {0};

	HAL_StatusTypeDef DMAstatus;
	size_t BytesWrittenSize = 0;

	//LoRaMessage[Size] = '\0';
	//BytesWrittenSize = snprintf(LoRaTxBuffer, sizeof(LoRaTxBuffer), "AT+UTX 5\rhello", Size, LoRaMessage);   //AT+UTX 5\rhello  "AT+SEND=2:0:%.*s\n"

    DMAstatus = HAL_UARTEx_ReceiveToIdle_DMA(&huart4, (uint8_t *)LoRaRxBuffer, LoRaRxBuf_SIZE);
	if (DMAstatus!= HAL_OK)
	{
		if(huart4.ReceptionType != HAL_UART_RECEPTION_TOIDLE)
		{
			printf("huart4.ReceptionType is set to 0; it will be set to 1 \n\r");

			huart4.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
		    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_IDLEF);
		     ATOMIC_SET_BIT(huart4.Instance->CR1, USART_CR1_IDLEIE);
		}
		else
		{
		  printf("DMA Error1 \n\r");
		}
	}

	__HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);

	//LoRaTxBuffer[50] = "AT+UTX 5\rhello";

	//if(HAL_UART_Transmit_DMA(&huart4, (uint8_t*)LoRaTxBuffer, BytesWrittenSize)!= HAL_OK)
    if(HAL_UART_Transmit_DMA(&huart4, (uint8_t*)buffer, 14)!= HAL_OK)
	{
		  printf("DMA Error2 \n\r");
		  return false;
	}

	while(LoRaDataReceivedFlag == false) {};

	LoRaDataReceivedFlag = false;

	return true;
}



void ProcessRxLoRaMsg(uint8_t *LoRaRxBuffer, uint16_t Size)
{
	uint8_t OkResponseStr1[] = "+OK\r\n\r\n";
	uint8_t OkResponseStr2[] = "\r\n\AT_ERROR\r\n";
	uint8_t DataSentReplyMsg1Str[] = "\r\n\AT_NO_NETWORK_JOINED\r\n";
	uint8_t  buffer[50] = {0};
	int result;


	printf("%s", (uint8_t *)LoRaRxBuffer);

	sprintf(buffer, "%s", (uint8_t *)LoRaRxBuffer);

	result = strncmp((char *)LoRaRxBuffer, (char *)DataSentReplyMsg1Str, Size);
	if (result == 0)
	{
		printf("Error Message: AT_NO_NETWORK_JOINED \n\r");
	}

	result = strncmp((char *)LoRaRxBuffer, (char *)OkResponseStr1, Size);
	if ((result == 0) && (LoRaMsgSent = false))
	{
		printf("AT Message: OK \n\r");
	}

	result = memcmp(buffer, (char *)OkResponseStr1, 3);
	if((LoRaMsgSent == true) && (result == 0))
	{
		printf("Message sent successfully \n\r");

		LoRaMsgSent = false;
	}

	result = strncmp((char *)LoRaRxBuffer, (char *)OkResponseStr2, Size);
	if (result == 0)
	{
		printf("AT Message: AT_ERROR\n\r");
	}

	memset(LoRaRxBuffer, 0, LoRaRxBuf_SIZE);
}



void QueryLoRaModule(void)
{
	  bool status = false;

	 //send uart tx to LORa Module
	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaATcommand, sizeof(LoRaATcommand));
	  if(status)
	  {
		  printf("Lora Module responded to AT command \n\n\r");
	  }

      // these can be uncommented as needed

//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetBaudRateATcommand, sizeof(LoRaGetBaudRateATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetVersionATcommand, sizeof(LoRaGetVersionATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetDevATcommand, sizeof(LoRaGetDevATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetBandATcommand, sizeof(LoRaGetBandATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaSetUS915BandATcommand, sizeof(LoRaSetUS915BandATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetClassATcommand, sizeof(LoRaGetClassATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetBandATcommand, sizeof(LoRaGetBandATcommand));
//
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetDevAddrATcommand, sizeof(LoRaGetDevAddrATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetDevEUIATcommand , sizeof(LoRaGetDevEUIATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetAppEUIATcommand , sizeof(LoRaGetAppEUIATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetNetworkSessionKeyATcommand , sizeof(LoRaGetNetworkSessionKeyATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetAppSessionKeyATcommand , sizeof(LoRaGetAppSessionKeyATcommand));
//
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaJoinATcommand , sizeof(LoRaJoinATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetRFParametersATcommand , sizeof(LoRaGetRFParametersATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetRFPowerATcommand , sizeof(LoRaGetRFPowerATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetDataRateATcommand, sizeof(LoRaGetDataRateATcommand));
//	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetRxChannelFreqATcommand , sizeof(LoRaGetRxChannelFreqATcommand));
	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetDataRateATcommand, sizeof(LoRaGetDataRateATcommand));

	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetPortATcommand , sizeof(LoRaGetPortATcommand));
	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaSendHelloMsgATcommand, sizeof(LoRaSendHelloMsgATcommand));
	  status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetRssiATcommand , sizeof(LoRaGetRssiATcommand));

}


void ReadRcvMsgRRSI(void)
{
   bool status = false;
   status = TestLoRaModuleComms(LoRaRxBuffer, LoRaGetRssiATcommand , sizeof(LoRaGetRssiATcommand));
}

void SendLoraMsg(void)
{
	bool status = false;

	LoRaMsgSent = true;

	// Turn ON the RF path for LORa RF path
    HAL_GPIO_WritePin(RF_SW_CNTRL_GPIO_Port, RF_SW_CNTRL_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    status = TestLoRaModuleComms(LoRaRxBuffer, LoRaSendHelloMsgATcommand, sizeof(LoRaSendHelloMsgATcommand));
}
