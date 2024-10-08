#ifndef  LoRa_H_
#define  LoRa_H_




#define LoRaRxBuf_SIZE     256


extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_uart4_tx;




bool TestLoRaModuleComms(volatile uint8_t *LoRaRxBuffer, const uint8_t *LoRaATcommand, uint8_t Size);

bool ProcessRxLoraDataForConsoleDisplay(uint8_t *LoRaRxBuffer, uint16_t Size);
bool SendLoRaDataPacket(uint8_t *LoRaMessage, uint8_t *LoRaRxBuffer, uint8_t Size);
void ProcessRxLoRaMsg(uint8_t *LoRaRxBuffer, uint16_t Size );
bool StartRSSIToneTest(uint8_t *LoRaRxBuffer);

void QueryLoRaModule(void);
void ReadRcvMsgRRSI(void);
void SendLoraMsg(void);
void SendLoraMsg(void);






#endif
