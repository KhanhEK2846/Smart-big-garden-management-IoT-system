#include "DataPackage.h"

DataPackage ::DataPackage()
{
    data = "";
    ID = "";
    Mode = "";
}

String DataPackage:: GetID() const
{
    return this->ID;
}

String DataPackage::GetData() const
{
    return this->data;
}

String DataPackage::GetMode() const
{
    return this->Mode;
}

void DataPackage::SetDataPackage(const String t_id, const String t_data, const String t_mode)
{
    this->ID = t_id;
    this->data = t_data;
    this->Mode = t_mode;
}

DataPackage ::~DataPackage()
{
    data.remove(0);
    ID.remove(0);
    Mode.remove(0);
}

DataPackage& DataPackage::operator=(const DataPackage temp)
{
    data = temp.data;
    ID = temp.ID;
    Mode = temp.Mode;
    return *this;
}

String DataPackage::toString() const
{
    String temp = String("{\n") + "ID: "+ this->ID+ "\nMode: "+ this->Mode+ "\nData: " + this->data +String("\n}");
    return temp;
};

void DataPackage::fromString(const String data)
{
    String temp = data.substring(data.indexOf("{\n") + 3, data.lastIndexOf("\n}"));
    this->ID = temp.substring(temp.indexOf("ID: ")+4,temp.indexOf("\nMode:"));
    this->Mode = temp.substring(temp.indexOf("Mode: ")+6,temp.indexOf("\nData:"));
    this->data = temp.substring(temp.indexOf("Data: ")+6, temp.length());
}