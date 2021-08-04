#include "Sock.hpp"
#include "Log.hpp"

class Util
{
public:
    //将字符串src按顺序分割成dst1 dst2 dst3三部分
    static void StringPart3(std::string &src,std::string &dst1,std::string &dst2,std::string &dst3)
    {
        //通过stringstream将字符串分割成三部分
        std::stringstream str(src); 

        str>>dst1>>dst2>>dst3;
    }

    static void StringPart2(std::string &src,std::string &dst1,std::string &dst2)
    {
        size_t i = src.find(':');
        if(i != std::string::npos)
        {
            dst1 = src.substr(0,i);
            dst2 = src.substr(i+2);
        }
    }
};
