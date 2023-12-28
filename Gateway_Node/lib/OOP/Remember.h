#include <stdlib.h>
#include <WString.h>

class Remember
{
private:
    String NodeAdrress[5];
    int count;
    int flag;
public:
    Remember();
    ~Remember();
    bool Add(const String Address = "");
    String GetAddrress(int index = -1);
};
