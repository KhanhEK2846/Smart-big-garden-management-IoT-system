#include "Transmit.h"

Transmit::Transmit()
{
    NextIP = "";
}

Transmit ::~Transmit()
{
    NextIP.remove(0);
}

void Transmit::SetNextIP(String dest)
{
    this->NextIP = dest;
}

String Transmit::GetNextIP() const
{
    return this->NextIP;
}

void Transmit::SetData(const DataPackage data)
{
    this->Data = data;
}

void Transmit::SetData (const String t_id, const String t_data)
{
    this->Data.SetDataPackage(t_id,t_data);
}

DataPackage Transmit::GetData() const
{
    return this->Data;
}

Transmit& Transmit::operator=(const Transmit temp)
{
    NextIP = temp.NextIP;
    Data = temp.Data;
    return *this;
}

void Transmit:: DataFromString(const String data)
{
    this->Data.fromString(data);
}