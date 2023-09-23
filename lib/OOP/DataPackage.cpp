#include "DataPackage.h"

DataPackage ::DataPackage()
{
    data = "";
    ID = "";
}

String DataPackage:: GetID() const
{
    return this->ID;
}

String DataPackage::GetData() const
{
    return this->data;
}

void DataPackage::SetDataPackage(const String t_id, const String t_data)
{
    this->ID = t_id;
    this->data = t_data;
}

DataPackage ::~DataPackage()
{
    data.remove(0);
    ID.remove(0);
}

DataPackage& DataPackage::operator=(const DataPackage temp)
{
    data = temp.data;
    ID = temp.ID;
    return *this;
}

String DataPackage::toString() const
{
    String temp = String("{\n") + "ID: "+ this->ID+ '\n' + "Data: " + this->data +String("\n}");
    return temp;
};

void DataPackage::fromString(const String data)
{
    String temp = data.substring(data.indexOf("{\n") + 3, data.lastIndexOf("\n}"));
    this->ID = temp.substring(temp.indexOf("ID: ")+4,temp.indexOf("\n"));
    this->data = temp.substring(temp.indexOf("Data: ")+6, temp.length());
}