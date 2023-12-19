#include "CommonFunction.h"

void CalculateAddressChannel(const String id, uint8_t &H, uint8_t &L, uint8_t &chan)
{
  int tempid[6];
  sscanf(id.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &tempid[0], &tempid[1], &tempid[2], &tempid[3], &tempid[4], &tempid[5]);
	H = tempid[0] + tempid[1] + tempid[2];
	L = tempid[3] + tempid[4] + tempid[5];
	chan = tempid[0] + tempid[1] + tempid[2] + tempid[3] + tempid[4] + tempid[5];
  while(H > 0xFF) H -= 0xFF;
  while(L > 0xFF) L -= 0xFF;
  while(chan > 0x1F) chan -= 0x1F;
}
String EnCodeAddressChannel(const uint8_t H,const uint8_t L,const uint8_t chan)
{
  char macStr[3] = { 0 };
  sprintf(macStr,"%02x%02x%02x", H,L,chan);
  return String(macStr);
}
void DeCodeAddressChannel(const String address, uint8_t &H, uint8_t &L, uint8_t &chan)
{
  sscanf(address.c_str(),"%02x%02x%02x",&H,&L,&chan);
}