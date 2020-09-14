#pragma once

//#include <Windows.h>
//#include<afxwin.h>


typedef unsigned char uint8_t;

typedef unsigned short uint16_t;

//typedef unsigned long uint32_t;

typedef int int32_t;



const uint16_t polynom = 0xA001;

uint16_t crc16table(uint8_t *ptr, uint16_t len);

uint16_t crc16tablefast(uint8_t *ptr, uint16_t len);

uint16_t crc16bitbybit(uint8_t *ptr, uint16_t len);

unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen);

void InvertUint8(unsigned char *dBuf, unsigned char *srcBuf);

void InvertUint16(unsigned short *dBuf, unsigned short *srcBuf);
