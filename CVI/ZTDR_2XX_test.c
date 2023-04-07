#include <stdio.h>
#include "ZTDR_2XX.h"

int main() {
	printf("Initializing ZTDR device...\n");
	int ret = ZTDR_Init();
	if (ret <= 0) {
		printf("ZTDR init failed with code %d\n", ret);
		ZTDR_CloseDevice();
		return ret;
	}
	printf("ZTDR device initialized successfully\n");
	
	ret = ZTDR_CalAmplitude ();
	
	ret = ZTDR_SetEnviron(2, 1, 0.0, 20.0, 2.25, 1024);
	printf("Acquiring data...\n");

	ret = ZTDR_AcquireData(1);
	if (ret <= 0) {
		printf("Acquistion failed with code %d\n", ret);
		ZTDR_CloseDevice();
		return ret;
	}
	
	/* ret = ZTDR_DumpFile ("data_".timestamp.".csv"); */

	ZTDR_CloseDevice();
	return 0;
}