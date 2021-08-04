#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

int main()
{
      std::string method;
      std::string query_string;
      if(getenv("METHOD"))
      {
          method = getenv("METHOD");
      }
      else
      {
          return 1;
      }

      if(strcasecmp(method.c_str(), "GET") == 0)
      {
          query_string = getenv("QUERY_STRING");
      }
      else if(strcasecmp(method.c_str(), "POST") == 0)
      {
          int cl = atoi(getenv("CONTENT-LENGTH"));
          char c = 0;
          while(cl)
          {
              read(0, &c, 1);
              query_string.push_back(c);
              cl--;
          }
      }

      //判断用户名和密码是否正确
      std::cout<<"success"<<std::endl;
      return 0;
}
