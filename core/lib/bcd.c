
#include "system.h"

int dec2bcd(int Decimal)
{
   return (((Decimal/10) << 4) | (Decimal % 10));
}

int bcd2dec(int BCD)
{
   return (((BCD>>4)*10) + (BCD & 0xF));
}
