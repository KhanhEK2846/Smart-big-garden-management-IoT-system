#include "Remember.h"

Remember::Remember()
{
    NodeAdrress[0] = "";
    NodeAdrress[1] = "";
    NodeAdrress[2] = "";
    NodeAdrress[3] = "";
    NodeAdrress[4] = "";
    count = 0;
    flag = 0;
}

Remember::~Remember()
{
    NodeAdrress[0].remove(0);
    NodeAdrress[1].remove(0);
    NodeAdrress[2].remove(0);
    NodeAdrress[3].remove(0);
    NodeAdrress[4].remove(0);
}

bool Remember::Add(const String Address)
{
    if( Address == "")
        return false;
    if(count == 5) //Overwrite
    {
        for(int i =0;i<5;i++)
        {
            if(this->NodeAdrress[i] == Address)
                break;
        }
        this->NodeAdrress[flag] = Address;
        flag = (flag>=4)? 0:flag+1;
    }
    else //Add
    {
        for(int i =0; i < 5;i++)
        {
            if(this->NodeAdrress[i] != "")
                continue;

            this->NodeAdrress[i] = Address;
            count = (count >= 5)? 5:count+1;
            break;
        }
    }

    return true;
}
