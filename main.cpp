#include "minios7.h"
#include <stdio.h>
#include <string.h>

#define EBUS 1
#define DUMP 2

#define SYN 0xAA

#define IO_BUF_SIZE 255
char ioBuf[IO_BUF_SIZE];

void printDump(const char* fmt, BYTE data);
void strDump(char* str);
void clearDump();

int MainQuit = 0;
//----------------------------------------------- main
void main()
{
  InitLib();
  LedOff();
  Led2Off();

  InstallCom(EBUS, 2400l, 8,0,1);
  InstallCom(DUMP, 115200l, 8,0,1);

  unsigned long t = GetTimeTicks();
  unsigned long dt;
  BYTE b;

  int a9seq = 0;
  int rcvMode = 0;
  int sndMode = 0;
  int bytes = 0;

  BYTE srcAddr;
  BYTE dstAddr;
  WORD cmd;

  for(;;)
  {
    // bytes 4-5 ms
    // SYN 40-50ms
    // AUTOSYN ?
    if ((GetTimeTicks() - t) > 500ul)
      LedOff();

    if (rcvMode)
      Led2On();
    else
      Led2Off();

		if (IsCom(DUMP))
		{
			b = ReadCom(DUMP);
			if (b == '1') sndMode = 1;
		}

    if (!IsCom(EBUS))
      continue;

    t = GetTimeTicks();
    LedOn();
    b = ReadCom(EBUS);

    if (b == 0xAA) // SYN
    {
      if (rcvMode != 0)
      {
      	if (/*srcAddr == 0x10*/ dstAddr==0x26 /* && !(dstAddr==0x3F || dstAddr==0x7F)*/ /*cmd==0xB504 && ioBuf[16] == '1' && ioBuf[17] == '6'*/) // <---- Filter here
      	{
	        strDump("\r\n");
  	      ToComStr(DUMP, ioBuf);
				}
        clearDump();
        rcvMode = 0;
      }
			if (sndMode == 1)
			{
				ioBuf[0] = 0x10;
				ioBuf[1] = 0x23;
				ioBuf[2] = 0x07;
				ioBuf[3] = 0x04;
				ioBuf[4] = 0x00;
				ioBuf[5] = 0x47;
				ioBuf[6] = '\0';
				ToComBufn(EBUS, ioBuf, 7);
				sndMode++;
				clearDump();
				rcvMode = 0;
			}
      continue;
    }
    else if (0xA9 == b)
    {
      a9seq = 1;
      continue;
    }
    else if (a9seq)
    {
      a9seq = 0;
      if (b == 0x00)
        b = 0xA9; 
      else if (b == 0x01)
        b = 0xAA;
    }

    switch (rcvMode)
    {
      case 0: // Master address
        printDump(">%02X", b);
        srcAddr = b;
        rcvMode++;
        break;

      case 1: // Slave address
        printDump(" %02X", b);
        dstAddr = b;
        rcvMode++;
        break;

      case 2: // Primary command
        printDump(" %02X", b);
        cmd = b;
        rcvMode++;
        break;

      case 3: // Secondary command
        printDump("%02X", b);
        cmd = (cmd << 8) | b;
        rcvMode++;
        break;

      case 4: // No of data bytes from master
      case 8: // No of data bytes from slave
        printDump(" %u [", b);
        bytes = b;
        rcvMode += (bytes ? 1 : 2);
        break;

      case 5: // Data bytes from master
      case 9: // Data bytes from slave
        printDump(" %02X", b);
        if (!--bytes) rcvMode++;
        break;

      case  6: // CRC from master
        printDump(" ] {%02X}", b);
        rcvMode++;
        break;

      case  7: // ACK from slave
        if (b == 0x00)
          strDump(" <ACK");
        else if (b == 0xFF)
          strDump(" <NAK");
        else
          printDump(" <?%02X", b);
        rcvMode++;
        break;

      case 10: // CRC from slave
        printDump(" ] {%02X}", b);
        rcvMode++;
        if (sndMode == 2)
        {
        	ToCom(EBUS, 0x00);
        	ToCom(EBUS, 0xAA);
        	sndMode = 0;
        }
        break;

      case 11: // ACK from master
        if (b == 0x00)
          strDump(" >ACK");
        else if (b == 0xFF)
          strDump(" >NAK");
        else
          printDump(" >?%02X", b);
        rcvMode++;
        break;

      default:
        printDump(" %02X", b);
        break;

    } // end switch
  } // end for(;;)
} // end main



void printDump(const char* fmt, BYTE data)
{
	char str[IO_BUF_SIZE];
	sprintf(str, fmt, data);
	strDump(str);
}

void strDump(char* str)
{
	strcat(ioBuf, str);
}

void clearDump()
{
	ioBuf[0] = '\0';
}
