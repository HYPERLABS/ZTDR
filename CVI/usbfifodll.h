






// FUNCTIONS TO VALIDATE BELOW
void usbfifo_close(void);
int usbfifo_isok(void);
void usbfifo_getid(char *buf, int len);
void usbfifo_getcomspd(char *buf, int len);
void usbfifo_devicereset(void);
int usbfifo_gethostbps(void);
void usbfifo_listdevs(void);

int usbfifo_checkmem(int *nlocs, int *failval);
int usbfifo_setparams(
	UINT8 freerun_en,
	UINT16 calstart,
	UINT16 calend,
	timeinf tmstart,
	timeinf tmend,
	UINT16 stepcount,
	UINT16 strobecount,
	UINT8 noversample,
	UINT16 record_len,
	UINT16 dac0,
	UINT16 dac1,
	UINT16 dac2 );

int usbfifo_acquire(UINT8 *ret_val, UINT8 arg);
int usbfifo_calibrate(UINT8 *ret_val, UINT8 arg);
int usbfifo_reset_calibrate(UINT8 *ret_val, UINT8 arg);
int usbfifo_readblock(UINT8 block_no, UINT16 *buf);
int usbfifo_command(UINT16 *ret_val, UINT8 cmd, UINT16 arg0, UINT16 arg1);
