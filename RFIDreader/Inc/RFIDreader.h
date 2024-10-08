/*
 * RFIDreader.h
 *
 *  Created on: Sep 20, 2024
 *      Author: fmabrouk
 */

#ifndef INC_RFIDREADER_H_
#define INC_RFIDREADER_H_




#include "main.h"
#include "string.h"
#include "stdbool.h"
#include <stdlib.h>
#include <stdio.h>



#define RxBuf_SIZE             2048
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))



//F U N C T I O N S ///
 void SetUpRFIDreader(void);
 void ScanForTags(void);
 void ProcessReceivedData(uint16_t Size);
 bool RFIDreaderSendCmd(volatile uint8_t *RFIDRxBuffer, const uint8_t *RFIDreaderTxCommand, uint8_t Size);
 uint16_t ReadRxDataSize(void);


















#endif /* INC_RFIDREADER_H_ */
