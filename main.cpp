#include "minios7.h"
#include <stdio.h>
#include <string.h>

#define EBUS 1
#define DUMP 2

#define DUMP_BUF_SIZE 255
char dumpBuf[DUMP_BUF_SIZE];

void printDump(char* fmt, BYTE data);
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
  int mode = 0;
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

    if (mode)
      Led2On();
    else
      Led2Off();

    if (!IsCom(EBUS))
      continue;

    t = GetTimeTicks();
    LedOn();
    b = ReadCom(EBUS);

    if (b == 0xAA)
    {
      if (mode != 0)
      {
      	if (1) // <---- Filter here
      	{
	        strDump("\r\n");
  	      ToComStr(DUMP, dumpBuf);
				}
        clearDump();
        mode = 0;
      }
      continue;
    }
    else if (b == 0xA9)
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

    switch (mode)
    {
      case 0: // Source address
        printDump(">%02X", b);
        srcAddr = b;
        mode++;
        break;

      case 1: // Destination address
        printDump(" %02X", b);
        dstAddr = b;
        mode++;
        break;

      case 2: // Primary command
        printDump(" %02X", b);
        cmd = b;
        mode++;
        break;

      case 3: // Secondary command
        printDump("%02X", b);
        cmd = (cmd << 8) | b;
        mode++;
        break;

      case 4: // No of data bytes from source
      case 8: // No of data bytes from destination
        printDump(" %u [", b);
        bytes = b;
        mode += (bytes ? 1 : 2);
        break;

      case 5: // Data bytes from source
      case 9: // Data bytes from destination
        printDump(" %02X", b);
        if (!--bytes) mode++;
        break;

      case  6: // CRC from source
      case 10: // CRC from destination
        printDump(" ] {%02X}", b);
        mode++;
        break;

      case  7: // ACK from destination
        if (b == 0x00)
          strDump(" <ACK");
        else if (b == 0xFF)
          strDump(" <NAK");
        else
          printDump(" <?%02X", b);
        mode++;
        break;

      case 11: // ACK from source
        if (b == 0x00)
          strDump(" >ACK");
        else if (b == 0xFF)
          strDump(" >NAK");
        else
          printDump(" >?%02X", b);
        mode++;
        break;

      default:
        printDump(" %02X", b);
        break;

    } // end switch
  } // end for(;;)
} // end main



void printDump(const char* fmt, BYTE data)
{
	char str[DUMP_BUF_SIZE];
	sprintf(str, fmt, data);
	strDump(str);
}

void strDump(char* str)
{
	strcat(dumpBuf, str);
}

void clearDump()
{
	dumpBuf[0] = '\0';
}
