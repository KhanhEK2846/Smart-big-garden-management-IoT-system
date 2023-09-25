#include "DataPackage.h"

class Transmit
{
private:
    String NextIP;
    DataPackage Data;
public:
    Transmit(/* args */);
    ~Transmit();
    Transmit& operator=(const Transmit temp);
    void SetNextIP(String dest);
    String GetNextIP() const;
    void SetData(const DataPackage data);
    void SetData (const String t_id, const String t_data);
    void DataFromString(const String data);
    DataPackage GetData() const;
};

