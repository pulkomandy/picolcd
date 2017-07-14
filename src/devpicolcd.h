/*
 * Library for using picolcd devices
 *
 * Written by Mandar Joshi 
 * License: LGPL
 *
 * 
 */

#ifndef DEVPICOLCD_H_
#define DEVPICOLCD_H_

#include <libusb-1.0/libusb.h>

#define MAX_LCD_DEVICES 2
#define INIT_DATA_LEN 24
#define KEY_INPUT 0x11
#define IR_INPUT 0x21
#define MINI_BOX 0x04d8
#define PICOLCD_20x2 0x0002
#define PICOLCD_20x4 0xc001
#define DEBUG

typedef struct MyLcdDevice MyLcdDevice;
typedef struct InputPacket InputPacket;
typedef struct IR20x4 IR20x4;
char *SpecialChar(int);

MyLcdDevice* picolcd_open(unsigned int);
int lcd_set_backlight(MyLcdDevice *,unsigned char);
int lcd_write_interrupt(MyLcdDevice *,unsigned char *,int);
InputPacket* lcd_read_input(MyLcdDevice *);

int lcd_20x2_init(MyLcdDevice *);
int lcd_20x2_display(MyLcdDevice *,int ,int,unsigned char *);
int lcd_20x2_clear(MyLcdDevice *);
int lcd_20x2_set_char (MyLcdDevice *, int , unsigned char *);
int lcd_20x2_lights(MyLcdDevice *, unsigned char , int);
short decode_rc5_20x2(unsigned char *,int);

int lcd_20x4_init(MyLcdDevice *);
int lcd_20x4_clear(MyLcdDevice *);
int lcd_20x4_display(MyLcdDevice *,int ,int,unsigned char *);
int lcd_20x4_set_char (MyLcdDevice *, int , unsigned char *);
int lcd_20x4_lights(MyLcdDevice *, unsigned char , int);
short decode_rc5_20x4(unsigned char *,int);
short decode_rc5_20x4_new(MyLcdDevice *,unsigned char *,int);

#pragma pack(0)
struct IR20x4 {
	short *irData;
	unsigned char *rc5Raw;		
	int irDataLen;
	unsigned char irCode[14];
	int rawBitCount,bitCount;
	unsigned int address;
	unsigned int command;
};

#pragma pack(0)
struct InputPacket {
	int inputType;
	int inputLen;
	unsigned char *inputData;
	int keyCode;
};

#pragma pack(0)
struct MyLcdDevice {
	unsigned short idVendor;
	unsigned short idProduct;
	unsigned char *deviceName;
	unsigned char initData[INIT_DATA_LEN];
	int maxLen;
	int irEnabled;
	void *irStruct;
	libusb_device *lcdDevice;
	libusb_device_handle *lcdHandle;
	int (*init_lcd)(MyLcdDevice *);
	int (*clear)(MyLcdDevice *);
	int (*display)(MyLcdDevice *,int, int, unsigned char *);
	int (*set_char)(MyLcdDevice *, int n, unsigned char *);
	InputPacket* (*read_input)(MyLcdDevice*);
	int (*write_interrupt)(MyLcdDevice *,unsigned char *,int);
	int (*set_backlight)(MyLcdDevice *,unsigned char);
	int (*set_lights)(MyLcdDevice *, unsigned char , int);
	short (*decode_ir)(MyLcdDevice*,unsigned char *,int);
	
};

static MyLcdDevice LcdDevices[MAX_LCD_DEVICES] = {
	{
		.idVendor = MINI_BOX,
		.idProduct = PICOLCD_20x2,
		.deviceName = "PicoLCD 20x2",
		.initData = {},
		.init_lcd = lcd_20x2_init,
		.maxLen = 24,
		.irEnabled = 1,
		.clear = lcd_20x2_clear,
  		.display = lcd_20x2_display,
 		.set_char = lcd_20x2_set_char,
		.read_input = lcd_read_input,
 		.write_interrupt = lcd_write_interrupt,
		.set_backlight = lcd_set_backlight,
 		.set_lights = lcd_20x2_lights,
 		.decode_ir = decode_rc5_20x4_new
	},
	{
		.idVendor = MINI_BOX, 
		.idProduct = PICOLCD_20x4,
		.deviceName = "PicoLCD 20x4",
		.initData = {0x94, 0x00, 0x07, 0x00, 0x32, 0x30, 0x00,0x32, 0x30, 0x00, 0x32,0x30, 0x00, 0x32, 0x38, 0x00, 0x32, 0x06, 0x00, 0x32, 0x0C, 0x07,0xD0, 0x01},
		.maxLen = 64,
		.irEnabled = 1,
		.init_lcd = lcd_20x4_init,
		.clear = lcd_20x4_clear,
 		.display = lcd_20x4_display,
 		.set_char = lcd_20x4_set_char,
 		.read_input = lcd_read_input,
 		.write_interrupt = lcd_write_interrupt,
		.set_backlight = lcd_set_backlight,
 		.set_lights = lcd_20x4_lights,
		.decode_ir = decode_rc5_20x4_new
	}
	
};

#endif
