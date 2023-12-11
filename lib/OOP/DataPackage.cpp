#include "DataPackage.h"

DataPackage ::DataPackage()
{
    data = "";
    ID = "";
    Mode = "";
    expired = 10;
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
    if(t_id != "")
        this->ID = t_id;
    if(t_data != "")
        this->data = t_data;
    if(t_mode != "")
        this->Mode = t_mode;
}

void DataPackage::SetMode(const String mode)
{
    this->Mode = mode;
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
    expired = temp.expired;
    return *this;
}

String DataPackage::toString(boolean prettier) const
{
    String temp;
    if(prettier)
        temp = "{\nID: ";
    else
        temp = "{";
    temp += this->ID;
    if(prettier)
        temp += "\nMode: ";
    else
        temp += ",";
    temp += this->Mode;
    if(prettier)
        temp += "\nData: ";
    else
        temp += "," ;
    temp += this->data ;
    if(prettier)
        temp += "\nExpired: ";
    else
        temp += "," ;
    temp += this->expired;
    if(prettier)
        temp += "\n}";
    else
        temp += "}";
    return temp;
};

void DataPackage::DataToJson(FirebaseJson* slave)
{
    String t_data = this->data;
    slave->set("Name",t_data.substring(0,t_data.indexOf("/")));//Name
    t_data = t_data.substring(t_data.indexOf("/")+1);
    int tempNumber = atof(t_data.substring(0,t_data.indexOf("/")).c_str());
    slave->set("Error/DHT11",String((tempNumber>>4)&1)); //Status DHT
    slave->set("Error/LDR",String((tempNumber>>3)&1));//Status Light Sensor
    slave->set("Error/Soil",String((tempNumber>>2)&1)); //Soid moisure sensor
    slave->set("Status/Led",String((tempNumber>>1)&1));//Light
    slave->set("Status/Pump",String((tempNumber>>0)&1));//Pump
    t_data = t_data.substring(t_data.indexOf("/")+1);
    slave->set("Sensor/DHT11/Humi",t_data.substring(0,t_data.indexOf("/")));//Humi
    t_data = t_data.substring(t_data.indexOf("/")+1);
    slave->set("Sensor/DHT11/Temp",t_data.substring(0,t_data.indexOf("/")));//Temperture
    t_data = t_data.substring(t_data.indexOf("/")+1);
    slave->set("Sensor/Light",t_data.substring(0,t_data.indexOf("/")));//Bright
    t_data = t_data.substring(t_data.indexOf("/")+1);
    slave->set("Sensor/Solid",t_data.substring(0,t_data.indexOf("/")));//Mois
    t_data = t_data.substring(t_data.indexOf("/")+1);
    slave->set("Plant/Days",t_data); // Day pass
    //slave->toString(t_data,true);
    //Serial.println(t_data);
}

void DataPackage::fromString(const String data)
{
    String temp = data.substring(data.indexOf("{") + 1, data.lastIndexOf("}"));
    this->ID = temp.substring(0,temp.indexOf(","));
    temp = temp.substring(temp.indexOf(",")+1);
    this->Mode = temp.substring(0,temp.indexOf(","));
    temp = temp.substring(temp.indexOf(",")+1);
    this->data = temp.substring(0,temp.indexOf(","));
    temp = temp.substring(temp.indexOf(",")+1);
    this->expired = atof(temp.c_str());
}