#ifndef MOCK_H
#define MOCK_H
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
 /*******************************************************************************
  * Definitions
  ******************************************************************************/
typedef struct {
	uint32_t record_type;
	uint32_t byte_count;
	uint8_t address[10];
	uint32_t addrLen;
	uint8_t data[256];
	uint8_t checksum[3];
	bool is_valid;
	uint8_t error_message[256];
} SRecord;
/*******************************************************************************
 * API
 ******************************************************************************/
void help();

bool input(uint32_t argc, uint8_t* argv[], FILE** input);

void Check_Record_Start(uint8_t* line, SRecord* record);

bool Check_S(uint8_t* line, SRecord* record, uint32_t S_123[]);

bool Check_Byte_Count(uint8_t* line, SRecord* record);

bool Check_Sum(uint8_t* line, SRecord* record);

bool check_Hexa(uint8_t* line, SRecord* record);

bool Check_Addr(uint8_t* line, SRecord* record);

void get_main_type(uint32_t* S_123, uint32_t* maintype);

void Check_Data_Record_and_Terminate(SRecord* records, uint32_t maintype, uint32_t rc_count);

void output(SRecord* records, FILE* Output, uint32_t rc_count);

#endif /* MOCK_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/