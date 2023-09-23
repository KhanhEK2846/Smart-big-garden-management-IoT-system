#include <stdlib.h>
#include <WString.h>

class DataPackage
{
private:
    String ID;
    String data;
public:
    DataPackage();
    String GetID() const;
    String GetData() const;
    void SetDataPackage(const String t_id, const String t_data);
    ~DataPackage();
    DataPackage& operator=(const DataPackage temp);
    String toString() const;
    void fromString(const String data);
};
