/*
 * Library for using picolcd devices
 *
 * Written by Mandar Joshi 
 * License: LGPL
 *
 * 
 */
#include "devpicolcd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * PicoLCD Open Function
 *
 * Opens Requested PicoLCD Device
 * @param whichDevice PICOLCD_20x2 or PICOLCD_20x4
 * @return An initialized MyLcdDevice on success. NULL on failure
 */
MyLcdDevice* picolcd_open (unsigned int whichDevice) {
	static libusb_context *ctx = NULL;

	MyLcdDevice *myLcdDevice;
	switch (whichDevice) {
		case PICOLCD_20x2:
			myLcdDevice = &LcdDevices[0];
			break;
		case PICOLCD_20x4:
			myLcdDevice = &LcdDevices[1];
			break;			
	}

	if (ctx == NULL) {
		printf("libusb_init %d\n",libusb_init(&ctx));
		libusb_set_debug(ctx,3);
	}
	/*	libusb_open(myLcdDevice->lcdDevice, &myLcdDevice->lcdHandle);*/
	myLcdDevice->lcdHandle = libusb_open_device_with_vid_pid (ctx, MINI_BOX, whichDevice);
	if (!myLcdDevice->lcdHandle) {
		puts("dev open failed");
		return NULL;
	}

	myLcdDevice->lcdDevice =  libusb_get_device (myLcdDevice->lcdHandle);
 	libusb_detach_kernel_driver(myLcdDevice->lcdHandle,0);
	libusb_claim_interface(myLcdDevice->lcdHandle,0);
	libusb_set_interface_alt_setting(myLcdDevice->lcdHandle,0,0);
	return myLcdDevice;
}
/**
 * PicoLCD 20x2 Initialization Function 
  *
 * The 20x2 doesn't need any initialization. This function just returns 0.
 * @param lcdDevice Pointer to the LcdDevice structure
  * @return  returns 0
 */
int lcd_20x2_init(MyLcdDevice *lcdDevice) {
	//libusb_clear_halt(lcdDevice->lcdHandle, LIBUSB_ENDPOINT_IN + 1);
	/* Initialize your decode stuff here*/
	IR20x4 *ir20x4;
	lcdDevice->irStruct=(void*)malloc(sizeof(IR20x4));		
	ir20x4=(IR20x4*)lcdDevice->irStruct;
	ir20x4->bitCount=0;	
	ir20x4->rawBitCount=0;		
	ir20x4->rc5Raw=(unsigned char *)malloc(128); /*assuming each element in irData is a LONG_PULSE (1 1)  or LONG_SPACE(0 0) */
	memset(ir20x4->rc5Raw,0,128);
	return 0;
}

/**
 * PicoLCD 20x4 Initialization Function 
 *
 * This initializes the 20x4 LCD.
 * @param lcdDevice Pointer to the LcdDevice structure
 * @return  returns 0
 */
int lcd_20x4_init(MyLcdDevice *lcdDevice) {
	lcdDevice->write_interrupt(lcdDevice,lcdDevice->initData,INIT_DATA_LEN);
	/* Initialize your decode stuff here*/
	IR20x4 *ir20x4;
	lcdDevice->irStruct=(void*)malloc(sizeof(IR20x4));		
	ir20x4=(IR20x4*)lcdDevice->irStruct;
	ir20x4->bitCount=0;	
	ir20x4->rawBitCount=0;		
	ir20x4->rc5Raw=(unsigned char *)malloc(128); /*assuming each element in irData is a LONG_PULSE (1 1)  or LONG_SPACE(0 0) */
	memset(ir20x4->rc5Raw,0,128);
	return 0;
}

/**
 * PicoLCD Set Backlight
 *
 * The sets the backlight brightness of the LcdDevice
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param value Ranges frorm 0x00 to 0xFF. Higher the value, higher the brightness.
 * @return  returns 0
 */
int lcd_set_backlight(MyLcdDevice *lcdDevice,unsigned char value) {
	unsigned char lcdPacket[2] = { 0x91 ,value};	
	lcdDevice->write_interrupt(lcdDevice,lcdPacket,2);
	return 0;
}

/**
 * PicoLCD Write Interrupt Function
 *
 * Does an interrupt write to the OUT Endpoint on the LcdDevice.
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param data Pointer to data to be sent to the LcdDevice
 * @param len Length of the data
 * @return  returns the number of bytes transferred to the LcdDevice
 */
int lcd_write_interrupt(MyLcdDevice *lcdDevice,unsigned char *data,int len) {
	int transferred;
	libusb_interrupt_transfer(lcdDevice->lcdHandle, LIBUSB_ENDPOINT_OUT + 1, (char *) data, len, &transferred,1000);
	return transferred;
}

/**
 * PicoLCD 20x4 Display Data Function
 *
 * Writes data to the 20x4 LcdDevice begining at the specified row and column
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param row Row Number
 * @param col Column Number
 * @param data Pointer to data to be sent to the LcdDevice. Should be NULL terminated
  * @return  returns 0
 */
int lcd_20x4_display(MyLcdDevice *lcdDevice,int row,int col,unsigned char *data)
{
        unsigned char lcdPacket[64]={0x95,0x01,0x00,0x01};
	int len, i, ret;
	int writeRow;
        unsigned char rowBytes[4][6] = {
                { 0x94, 0x00, 0x01, 0x00, 0x64, 0x80+col },
                { 0x94, 0x00, 0x01, 0x00, 0x64, 0xC0+col },
                { 0x94, 0x00, 0x01, 0x00, 0x64, 0x94+col },
                { 0x94, 0x00, 0x01, 0x00, 0x64, 0xD4+col }
        };

	writeRow=row>3?0:row;
    	ret = lcdDevice->write_interrupt(lcdDevice,rowBytes[writeRow],6);
        len = strlen((char *)data);
        if(len+col>20)
		len=20-col;
        lcdPacket[4] = len;
	memcpy(lcdPacket+5,data,len);
        lcdDevice->write_interrupt(lcdDevice, lcdPacket, 5+len);
	return ret;
}

/**
 * PicoLCD 20x2 Display Data Function
 *
 * Writes data to the 20x2 LcdDevice begining at the specified row and column
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param row Row Number
 * @param col Column Number
 * @param data Pointer to data to be sent to the LcdDevice. Should be NULL terminated
 * @return  returns 0
 */
int lcd_20x2_display(MyLcdDevice *lcdDevice,int row,int col,unsigned char *data)
{
        int len, ret;
        len = strlen((char *)data);
        if (len+col > 20) 
		len = 20-col;

	unsigned char lcdPacket[64]={0x98,row,col,len};
	memcpy(lcdPacket+4,data,len);
	ret = lcdDevice->write_interrupt(lcdDevice, lcdPacket, 4+len);
	
	return ret;
}

/**
 * PicoLCD 20x2 Clear Screen Function
 *
 * This clears the 20x2 screen 
 * @param lcdDevice Pointer to the LcdDevice structure
 * @return  returns 0
 */
int lcd_20x2_clear(MyLcdDevice *lcdDevice) {
	lcdDevice->display(lcdDevice,0,0,"                    ");
	lcdDevice->display(lcdDevice,1,0,"                    ");
	return 0;
}

/**
 * PicoLCD 20x4 Clear Screen Function
 *
 * This clears the 20x4 screen 
 * @param lcdDevice Pointer to the LcdDevice structure
 * @return  returns 0
 */
int lcd_20x4_clear(MyLcdDevice *lcdDevice) {
	lcdDevice->display(lcdDevice,0,0,"                    ");
	lcdDevice->display(lcdDevice,1,0,"                    ");
	lcdDevice->display(lcdDevice,2,0,"                    ");
	lcdDevice->display(lcdDevice,3,0,"                    ");
	return 0;
}

/**
 * PicoLCD 20x4 Set Char Function
 *
 * Sets the special character (1 -7) on the 20x4 LcdDevice
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param n Character Number (1 -7)
 * @param charData A 8 byte array. Each byte describes a 5 bit row
 * @return  returns 0
 */
int lcd_20x4_set_char (MyLcdDevice *lcdDevice, int n, unsigned char *charData)
{

	unsigned char setCharCmd[6] = { 0x94, 0x00, 0x01, 0x00, 0x64, 0x40+8*n }; 
	unsigned char setCharData[13] = { 0x95, 0x01, 0x00, 0x01, 0x08,
					charData[0], charData[1], charData[2], charData[3], 
  					charData[4], charData[5], charData[6], charData[7]};
	
	lcdDevice->write_interrupt(lcdDevice, setCharCmd, 6);
	lcdDevice->write_interrupt(lcdDevice, setCharData, 13);
	return 0;
}

/**
 * PicoLCD 20x2 Set Char Function
 *
 * Sets the special character (1 -7) on the 20x2 LcdDevice
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param n Character Number (1 -7)
 * @param charData A 8 byte array. Each byte describes a 5 bit row
 * @return  returns 0
 */
int lcd_20x2_set_char (MyLcdDevice *lcdDevice, int n, unsigned char *charData)
{
	unsigned char lcdPacket[10] = { 0x9c,n,
					charData[0], charData[1], charData[2], charData[3],
					charData[4],charData[5],charData[6],charData[7] };	
	lcdDevice->write_interrupt(lcdDevice, lcdPacket, 10);	
	return 0;
}

/**
 * PicoLCD 20x2 Set Key Lights Function
 *
 * Switches on or Switches off Lights on the Keys of the 20x2 LcdDevice
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param lightSet 1 Byte data describing which lights to switch on
 * @param state 1 to set the lights as per lightSet. 0 to switch them off
 * @return  returns 0
 */
int lcd_20x2_lights(MyLcdDevice *lcdDevice, unsigned char lightSet, int state) {
	unsigned char lcdPacket[]={0x81,0xFF};
 	lcdPacket[1]=(state==0)?0:lightSet;
	lcdDevice->write_interrupt(lcdDevice,lcdPacket,2);
	return 0;
}

/**
 * PicoLCD 20x4 Set Lights Function
 *
 * The 20x4 LcdDevice does not have key lights. This function just returns 0.
 * @param lcdDevice Pointer to the LcdDevice structure
 * @param lightSet 1 Byte data describing which lights to switch on
 * @param state 1 to set the lights as per lightSet. 0 to switch them off
 * @return  returns 0
 */
int lcd_20x4_lights(MyLcdDevice *lcdDevice, unsigned char lightSet, int state) {
	return 0;
}

/**
 * Get a 2 byte null terminated array having the first byte as n
 *
 * This function is used to get the data required to display a special character
  * @param n Special Character index (1 - 7)
  * @return  returns a 2 byte null terminated array having the first byte as n
 */
char * SpecialChar(int n) {
	char *specialChar;
	specialChar=(char *)malloc(2);
	specialChar[0]=(char)n;
	specialChar[1]=0;
	return specialChar;
}

/**
 * PicoLCD Read Input Function
 *
 * Reads input from user (IR and Key Press)
 * @param lcdDevice Pointer to the LcdDevice structure
 * @return  returns an InputPacket which contains inputType, inputLen,inputData and keyCode;
 */
InputPacket* lcd_read_input(MyLcdDevice *lcdDevice) {
	int ret=0,transferred;
	unsigned char *recvBytes;
	InputPacket *inputPacket;
	inputPacket=(InputPacket*)malloc(sizeof(InputPacket));
	inputPacket->inputData=(unsigned char*)malloc(lcdDevice->maxLen);
	recvBytes=(unsigned char*)malloc(lcdDevice->maxLen+2);
	int keyCode=-1,count=0;

	while(1) {
		ret = libusb_interrupt_transfer(lcdDevice->lcdHandle, LIBUSB_ENDPOINT_IN + 1, recvBytes, lcdDevice->maxLen, &transferred,0xFFFF);
		
//  		printf("recvBytes: %d %02X %02X %02X\n",transferred,recvBytes[0],recvBytes[1],recvBytes[2]);	
		if(ret==0) {
			switch(recvBytes[0]) {
				case KEY_INPUT:
// 					libusb_clear_halt(lcdDevice->lcdHandle, LIBUSB_ENDPOINT_IN + 1);
					if(recvBytes[1]==0 && recvBytes[2]==0 && keyCode!=-1) {
						inputPacket->inputType=KEY_INPUT;
						inputPacket->keyCode=keyCode;						
 						return inputPacket;
					}
					keyCode=recvBytes[1];
					break;
				case IR_INPUT:
// 					libusb_clear_halt(lcdDevice->lcdHandle, LIBUSB_ENDPOINT_IN + 1);
					if(lcdDevice->irEnabled) {
						inputPacket->inputType=IR_INPUT;
						inputPacket->inputLen=recvBytes[1];
						inputPacket->inputData=recvBytes+2;	
						return inputPacket;
					}
					break;
			}
		}
		
	count++;	
	}
	
}

/**
 * PicoLCD 20x2 IR Decode function for Philips RC5 Remotes
 *
 * Decodes IR Data 
 * @param irData Pointer to the received IR data 
 * @param len Length of the received IR data
 * @return  returns address and command. address=(return >> 6)&0x1F. command=return&0x3F
 */
short decode_rc5_20x2(unsigned char *irData,int len) {
	return 0; //decode_rc5_20x4_new(irData,len);
}


/**
 * PicoLCD 20x4 IR Decode function for Philips RC5 Remotes
 *
 * Decodes IR Data 
 * @param irData Pointer to the received IR data 
 * @param len Length of the received IR data
 * @return  returns a short int. The binary format is 00<start-2-bits>0<address-5-bits><command-6-bits>
 */
short decode_rc5_20x4_new(MyLcdDevice *lcdDevice,unsigned char *irData,int len) {
	int i;
	short data;
	IR20x4 *ir20x4=(IR20x4*)lcdDevice->irStruct;
	ir20x4->irData=(short *)irData;	
	ir20x4->irDataLen=len/2;

	for(i=0;i<ir20x4->irDataLen;i++) {
		data=-ir20x4->irData[i];
		if(data<0) {
			data=abs(data);
			if(data> 444 && data < 1333) {
				
			ir20x4->rc5Raw[ir20x4->rawBitCount++]=0;
			}
			else if(data > 1332 && data < 2222) {
				
				ir20x4->rc5Raw[ir20x4->rawBitCount++]=0;
				ir20x4->rc5Raw[ir20x4->rawBitCount++]=0;
			}
			else {
				ir20x4->bitCount=0;
				ir20x4->rawBitCount=0;
				return -1;
			}
		}
		else {
			if(data> 444 && data < 1333) {
				ir20x4->rc5Raw[ir20x4->rawBitCount++]=1;
				
			}
			else if(data > 1332 && data < 2222) {
				ir20x4->rc5Raw[ir20x4->rawBitCount++]=1;
				ir20x4->rc5Raw[ir20x4->rawBitCount++]=1;
				
			}
			else {
				ir20x4->bitCount=0;
				ir20x4->rawBitCount=0;
				return -1;
			}
		}

	}

	ir20x4->bitCount=0;
 	ir20x4->irCode[ir20x4->bitCount++]=1; /*it will always begins with 1 [at least with the Philips remote I have]*/	
 	i=1;  /* we skip the first bit as it is always 1 [at least with the Philips remote I have]*/
	

	for(i=1;i<ir20x4->rawBitCount-1;i+=2) {
		if(ir20x4->rc5Raw[i]==0 && ir20x4->rc5Raw[i+1]==1) {
			ir20x4->irCode[ir20x4->bitCount++]=1;
		}
		else if(ir20x4->rc5Raw[i]==1 && ir20x4->rc5Raw[i+1]==0) {
			ir20x4->irCode[ir20x4->bitCount++]=0;
		}
		else {
			ir20x4->bitCount=0;
			ir20x4->rawBitCount=0;
			return -1;
		}
	}
	
	if(ir20x4->rawBitCount<0x1A) {
		return -1;		
	}

	if(ir20x4->rc5Raw[25]==0) {
		ir20x4->irCode[ir20x4->bitCount++]=1;
	}
	else if(ir20x4->rc5Raw[25]==1) {

		ir20x4->irCode[ir20x4->bitCount++]=0;
	}	
	

	ir20x4->address=(ir20x4->irCode[3]<<4)+(ir20x4->irCode[4]<<3)+(ir20x4->irCode[5]<<2)+(ir20x4->irCode[6]<<1)+(ir20x4->irCode[7]);
	ir20x4->command=(ir20x4->irCode[8]<<5)+(ir20x4->irCode[9]<<4)+(ir20x4->irCode[10]<<3)+(ir20x4->irCode[11]<<2)+(ir20x4->irCode[12]<<1)+(ir20x4->irCode[13]);
	
	ir20x4->rawBitCount=0;
	ir20x4->bitCount=0;
	return (ir20x4->irCode[0]<<13)+(ir20x4->irCode[1]<<12)+(ir20x4->address << 6) + ir20x4->command;
}

