#include <stdlib.h>
#include <WString.h>

class DataPackage
{
private:
    String Mode;
    String ID;
    String data;
public:
    DataPackage();
    String GetID() const;
    String GetData() const;
    String GetMode() const;
    void SetDataPackage(const String t_id, const String t_data, const String t_mode);
    ~DataPackage();
    DataPackage& operator=(const DataPackage temp);
    String toString() const;
    void fromString(const String data);
};
