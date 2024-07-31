#define _CRT_SECURE_NO_WARNINGS
/*******************************************************************************
 * Includes
 *******************************************************************************/
#include "mock.h"

 /*******************************************************************************
  * Functions
  *******************************************************************************/

void help() {
	printf("This program parses an S-record format file and generates an output file named 'Output.txt'.\n");
	printf("Usage: ./Mock <path_to_srecord_file>\n");
	printf("The output file contains lines composed of the order number, Address, and Data, separated by spaces.\n");
	printf("Errors in the S-record file are printed to the screen with details.\n");
}

bool input(uint32_t argc, uint8_t* argv[], FILE** input_file)
{
	bool checkinput;
	checkinput = false;
	if (argc != 2) {
		help();
	}
	else {
		*input_file = fopen(argv[1], "r");
		checkinput = true;
		if (!input_file) {
			printf("File not found");
			checkinput = false;
		}
	}
	return checkinput;
}

void Check_Record_Start(uint8_t* line, SRecord* record) {
	bool is_valid = true;
	uint8_t error_message[256] = "";
	if (line[1] != '0') {
		is_valid = false;
		sprintf(error_message, "Line does not start with 'S0', found 'S%c' ", line[1]);
		strcat(record->error_message, error_message);
	}
	record->is_valid = is_valid;
}

bool Check_S(uint8_t* line, SRecord* record, uint32_t S_123[]) {
	bool is_valid = true;
	uint8_t error_message[256] = "";

	if (line[0] != 'S') {
		is_valid = false;
		strcat(error_message, "Line does not start with 'S'. ");
	}

	uint32_t record_type = (uint32_t)(line[1] - '0');
	record->record_type = record_type;
	if (record_type > 9 || record_type == 4) {
		is_valid = false;
		char type_error[50];
		sprintf(type_error, "Invalid record type: %d. ", record_type);
		strcat(error_message, type_error);
	}
	else {
		if (record_type >= 1 && record_type <= 3) {
			S_123[record_type]++;
		}
	}

	if (strlen(error_message) > 0) {
		strcat(record->error_message, error_message);
	}

	record->is_valid = is_valid && record->is_valid; /* Keep the overall validity status */
	return is_valid;
}

bool Check_Byte_Count(uint8_t* line, SRecord* record) {
	bool is_valid = true;
	uint8_t error_message[256] = "";

	uint8_t byte_count_str[3] = { line[2], line[3], '\0' };
	uint32_t byte_count = strtoul(byte_count_str, NULL, 16);

	uint32_t real_byte_count = (uint32_t)((strlen(line) - 5) / 2);

	if (byte_count < 3 || byte_count > 255) {
		is_valid = false;
		char byte_count_error[50];
		sprintf(byte_count_error, "Invalid byte count: %s. ", byte_count_str);
		strcat(error_message, byte_count_error);
	}
	else
	{
		if (byte_count != real_byte_count) {
			is_valid = false;
			char length_error[50];
			sprintf(length_error, "Line length does not match byte count: %s. ", byte_count_str);
			strcat(error_message, length_error);
		}
		else {
			record->byte_count = byte_count;
		}
	}
	if (strlen(error_message) > 0) {
		strcat(record->error_message, error_message);
	}

	record->is_valid = is_valid && record->is_valid; /* Keep the overall validity status */
	return is_valid;
}

bool check_Hexa(uint8_t* line, SRecord* record) {
	bool is_valid = true;
	uint8_t error_message[256] = "";
	uint32_t i;


	for (i = 1; i < strlen((uint8_t*)line) - 1; i++) {
		if (!isxdigit(line[i])) {
			is_valid = false;
			char hexa_error[50];
			sprintf(hexa_error, "Invalid hex character at position %d: %c. ", i, line[i]);
			strcat(error_message, hexa_error);
		}
	}

	if (strlen(error_message) > 0) {
		strcat(record->error_message, error_message);
	}

	record->is_valid = is_valid && record->is_valid;
	return is_valid;
}

bool Check_Addr(uint8_t* line, SRecord* record) {
	bool is_valid = true;
	uint8_t error_message[256] = "";
	uint8_t data[256];
	uint32_t lineLength;
	uint32_t i;

	strcpy(data, "");
	lineLength = strlen(line) - 1;
	if (record->record_type == 0 || record->record_type == 1 || record->record_type == 5 || record->record_type == 9) {
		for (i = 4; i <= 7; i++) {
			strncat(record->address, &line[i], 1);
			record->addrLen = 4;
		}
		for (i = 8; i < lineLength - 2; i++)
		{
			strncat(record->data, &line[i], 1);
		}
	}
	else if (record->record_type == 2 || record->record_type == 8 || record->record_type == 6) {
		for (i = 4; i <= 9; i++) {
			strncat(record->address, &line[i], 1);
			record->addrLen = 6;
		}
		for (i = 10; i < lineLength - 2; i++)
		{
			strncat(record->data, &line[i], 1);
		}
	}
	else {
		for (i = 4; i <= 11; i++) {
			strncat(record->address, &line[i], 1);
			record->addrLen = 8;
		}
		for (i = 12; i < lineLength - 2; i++)
		{
			strncat(record->data, &line[i], 1);
		}
	}
	return is_valid;
}

bool Check_Sum(uint8_t* line, SRecord* record) {
	bool is_valid = true;
	uint8_t error_message[256] = "";
	uint32_t lsb;
	uint32_t real_checksum;
	uint32_t i;
	size_t len = strlen((uint8_t*)line) - 1;
	uint32_t sum = 0;
	uint8_t checksum_str[3] = { line[len - 2], line[len - 1], '\0' };
	uint32_t checksum = (uint32_t)strtoul(checksum_str, NULL, 16);

	for (i = 2; i < len - 2; i += 2) {
		uint8_t byte_str[3] = { line[i], line[i + 1], '\0' };
		uint32_t byte_value = (uint32_t)strtoul(byte_str, NULL, 16);
		sum += byte_value;
	}

	lsb = sum & 0xFF;

	real_checksum = 0xFF - lsb;
	if (real_checksum != checksum) {
		is_valid = false;
		strcat(error_message, "Ivalid check sum. ");
	}
	else
	{
		strcat(record->checksum, checksum_str);
	}
	if (strlen(error_message) > 0) {
		strcat(record->error_message, error_message);
	}
	record->is_valid = is_valid && record->is_valid;
	return is_valid;
}

void get_main_type(uint32_t* S_123, uint32_t* maintype)
{
	uint32_t i;
	uint32_t max;
	max = 0;
	*maintype = 0;
	for (i = 1; i < 4; i++) {
		if (S_123[i] > max) {
			max = S_123[i];
			*maintype = i;
		}
	}
}

void Check_Data_Record_and_Terminate(SRecord* records, uint32_t maintype, uint32_t rc_count)
{
	uint8_t data_message[256] = "";
	uint8_t terminate_message[256] = "";
	uint32_t terminate;
	terminate = 10 - maintype;
	uint32_t i;
	for (i = 1; i < rc_count - 1; i++)
	{
		if (records[i].record_type != maintype && records[i].record_type != 5 && records[i].record_type != 6) {
			char type_error[70];
			sprintf(type_error, "Invalid record type, the record type that appears most often is S%d. ", maintype);
			strcat(data_message, type_error);
			if (strlen(data_message) > 0) {
				strcat(records[i].error_message, data_message);
			}
			records[i].is_valid = false;
		}
	}
	if (records[rc_count - 1].record_type != terminate) {
		char type_error[50];
		sprintf(type_error, "Invalid record type, the terminate type is S%d. ", terminate);
		strcat(terminate_message, type_error);
		if (strlen(terminate_message) > 0) {
			strcat(records[i].error_message, terminate_message);
		}
		records[i].is_valid = false;
	}
}

void output(SRecord* records, FILE* Output, uint32_t rc_count)
{
	uint32_t i;
	fprintf(Output, "Index  Type    Byte_counts     Address                Data                                                         Checksum \n");
	for (i = 0; i < rc_count; i++) {
		if (records[i].is_valid == true) {
			if (strlen(records[i].data) != 0) {
				fprintf(Output, "%-5d   S%-5d   0x%-10X   0x%-15s   0x%-60s   0x%s\n", i, records[i].record_type, records[i].byte_count, records[i].address, records[i].data, records[i].checksum);
			}
			else
			{
				fprintf(Output, "%-5d   S%-5d   0x%-10X   0x%-15s                                                                    0x%s\n", i, records[i].record_type, records[i].byte_count, records[i].address, records[i].checksum);

			}
		}
		else
		{
			fprintf(Output, "%-5d   %s\n", i, records[i].error_message);
			printf("%-5d   %s\n", i, records[i].error_message);
		}
	}
}
/*******************************************************************************
 * EOF
 ******************************************************************************/