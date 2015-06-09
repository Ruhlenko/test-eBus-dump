#include "minios7.h"

int MainQuit = 0;
//----------------------------------------------- main
void main()
{
  InitLib();
  LedOff();
  Led2Off();

  InstallCom(1, 2400l, 8,0,1);
  InstallCom(2, 115200l, 8,0,1);

  unsigned long t = GetTimeTicks();
  unsigned long dt;
  unsigned char b;

  int iscmd = 0;

  for(;;)
  {
    if (IsCom(1))
    {
    	// SYN 40-50ms
    	// bytes 4-5 ms
    	t = GetTimeTicks();
    	b = ReadCom(1);

    	if (b == 0xAA) // SYN
    	{
	    	LedOn();
	    	if (iscmd)
				{
					iscmd = 0;
					Led2Off();
					ToComStr(2, "\r\n");
				}
    	}
    	else
    	{
    		Led2On();
    		iscmd = 1;
				printCom(2, "%02X ", b);
			}
		}
		else
		{
			dt = GetTimeTicks() - t;
			if (dt > 100ul)
			{
				LedOff();
			}
//			else if (iscmd && dt > 10ul)
//			{
//				iscmd = 0;
//				Led2Off();
//				ToComStr(2, "\r\n");
//			}
		}

  }
}
