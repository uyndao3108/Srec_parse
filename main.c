#define _CRT_SECURE_NO_WARNINGS
/*******************************************************************************
 * Includes
 *******************************************************************************/
#include "mock.h"
 /*******************************************************************************
  * Main
  *******************************************************************************/
uint32_t main(uint32_t argc, uint8_t* argv[]) {
	FILE* input_file;
	FILE* output_file;
	SRecord records[1350];
	uint32_t rc_count;
	bool check_init;
	bool checkS;
	bool checkHexa;
	bool checkByteCount;
	bool checkAddr;
	bool checkSum;
	uint32_t S_123[4] = { 0 };
	uint32_t* maintype;
	uint8_t line[1350];

	rc_count = 0;
	check_init = 0;
	checkS = 0;
	checkHexa = 0;
	checkAddr = 0;
	checkSum = 0;
	input_file = NULL;
	output_file = NULL;

	if (input(argc, argv, &input_file)) {
		while (fgets(line, sizeof(line), input_file))
		{
			SRecord record = { .address = "", .data = "", .checksum = "", .is_valid = true, .error_message = "" };

			if (!check_init) {
				Check_Record_Start(line, &record);
				check_init = 1;
			}

			checkS = Check_S(line, &record, S_123);

			if (checkS = true) {
				checkHexa = check_Hexa(line, &record);
			}
			if (checkHexa = true) {
				checkByteCount = Check_Byte_Count(line, &record);
			}
			if (checkByteCount = true) {
				checkAddr = Check_Addr(line, &record);
			}
			if (checkAddr = true) {
				checkSum = Check_Sum(line, &record);
			}
			records[rc_count] = record;
			rc_count++;
		}
		fclose(input_file);
		get_main_type(S_123, &maintype); /* Get the record type that appears most often */
		Check_Data_Record_and_Terminate(records, maintype, rc_count);
		output_file = fopen("Output.txt", "w");
		if (output_file == NULL) {
			printf("Opening the file for writing failed. ");
		}
		else
		{
			output(records, output_file, rc_count);
		}
	}
	return 0;
}
/*******************************************************************************
 * EOF
 ******************************************************************************/