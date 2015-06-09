#include "minios7.h"

#define EBUS 1
#define DUMP 2

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
  unsigned char b;

  int a9seq = 0;
  int mode = 0;
  int bytes = 0;

  for(;;)
  {
    // SYN 40-50ms
    // bytes 4-5 ms
    if ((GetTimeTicks() - t) > 100ul)
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
        mode = 0;
        ToComStr(DUMP, "\r\n");
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
      case 1: // Source address
        printCom(DUMP, ">%02X", b);
        mode++;
        break;

      case 2: // Destination address
      case 3: // Primary command
        printCom(DUMP, " %02X", b);
        mode++;
        break;

      case 4: // Secondary command
        printCom(DUMP, "%02X", b);
        mode++;
        break;

      case 5: // No of data bytes from source
      case 9: // No of data bytes from destination
        printCom(DUMP, " %u [", b);
        bytes = b;
        mode += (bytes ? 1 : 2);
        break;

      case  6: // Data bytes from source
      case 10: // Data bytes from destination
        printCom(DUMP, " %02X", b);
        if (!--bytes) mode++;
        break;

      case  7: // CRC from source
      case 11: // CRC from destination
        printCom(DUMP, " ] {%02X}", b);
        mode++;
        break;

      case  8: // ACK from destination
        if (b == 0x00)
          ToComStr(DUMP, " <ACK");
        else if (b == 0xFF)
          ToComStr(DUMP, " <NAK");
        else
          printCom(DUMP, " <?%02X", b);
        mode++;
        break;

      case 12: // ACK from source
        if (b == 0x00)
          ToComStr(DUMP, " >ACK");
        else if (b == 0xFF)
          ToComStr(DUMP, " >NAK");
        else
          printCom(DUMP, " >?%02X", b);
        mode++;
        break;

      default:
        printCom(DUMP, " %02X", b);
        break;

    } // end switch
  } // end for(;;)
} // end main
