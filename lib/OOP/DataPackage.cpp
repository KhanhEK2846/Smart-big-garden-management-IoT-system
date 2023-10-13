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
    return *this;
}

String DataPackage::toString() const
{
    String temp = String("{\n") ;
    temp += "ID: ";
    temp += this->ID;
    temp += "\nMode: ";
    temp += this->Mode;
    temp += "\nData: " ;
    temp += this->data ;
    temp +=String("\n}");
    return temp;
};

void DataPackage::DataToJson(FirebaseJson* slave)
{
    String t_data = this->data;
    slave->set("Name",t_data.substring(t_data.indexOf("/")+1,t_data.indexOf("/StatusD")));//Name
    slave->set("Error/DHT11",t_data.substring(t_data.indexOf("StatusD ")+8,t_data.indexOf("/StatusL"))); //Status DHT
    slave->set("Error/LDR",t_data.substring(t_data.indexOf("StatusL ")+8,t_data.indexOf("/StatusM")));//Status Light Sensor
    slave->set("Error/Soil",t_data.substring(t_data.indexOf("StatusM ")+8,t_data.indexOf("/Day"))); //Soid moisure sensor
    slave->set("Plant/Days",t_data.substring(t_data.indexOf("Day ")+4,t_data.indexOf("/humi"))); // Day pass
    slave->set("Sensor/DHT11/Humi",t_data.substring(t_data.indexOf("humi ")+5,t_data.indexOf("/temp")));//Humi
    slave->set("Sensor/DHT11/Temp",t_data.substring(t_data.indexOf("temp ")+5,t_data.indexOf("/ligh")));//Temperture
    slave->set("Sensor/Light",t_data.substring(t_data.indexOf("ligh ")+5,t_data.indexOf("/mois")));//Bright
    slave->set("Sensor/Solid",t_data.substring(t_data.indexOf("mois ")+5,t_data.indexOf("/LED")));//Mois
    slave->set("Status/Led",t_data.substring(t_data.indexOf("LED ")+4,t_data.indexOf("/Pump")));
    slave->set("Status/Pump",t_data.substring(t_data.indexOf("Pump ")+5,t_data.indexOf("/MQTT")));
    //slave->toString(t_data,true);
    //Serial.println(t_data);
}

void DataPackage::fromString(const String data)
{
    String temp = data.substring(data.indexOf("{\n") + 3, data.lastIndexOf("\n}"));
    this->ID = temp.substring(temp.indexOf("ID: ")+4,temp.indexOf("\nMode:")-1);
    this->Mode = temp.substring(temp.indexOf("Mode: ")+6,temp.indexOf("\nData:")-1);
    this->data = temp.substring(temp.indexOf("Data: ")+6, temp.length()-1);
}