#include <stdlib.h>
#include <WString.h>
#include <FirebaseESP32.h>
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
    void SetMode(const String mode);
    ~DataPackage();
    DataPackage& operator=(const DataPackage temp);
    String toString() const;
    void DataToJson(FirebaseJson * slave);
    void fromString(const String data);
};
