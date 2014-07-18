typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

//#define BIG_ENDIAN 1

struct _delay16
{
#ifdef BIG_ENDIAN
	UINT16 int_val;
	UINT16 frac_val;
#else
	UINT16 frac_val;
	UINT16 int_val;
#endif
};

struct _delay8
{
#ifdef BIG_ENDIAN
	UINT8 b3;
	UINT8 b2;
	UINT8 b1;
	UINT8 b0;
#else
	UINT8 b0;
	UINT8 b1;
	UINT8 b2;
	UINT8 b3;
#endif
};

union _timeinf
{
	UINT32 time;
	struct _delay16 time_s;
	struct _delay8 time_b;
};

typedef union _timeinf timeinf;


int usbfifo_open(void);
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
