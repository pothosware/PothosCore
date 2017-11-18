//used by poco when building it as a submodule
//this is an open issue, this file can be deleted after:
//https://github.com/pocoproject/poco/pull/1991

#include <iostream>
#include <string>

int main()
{
  std::string str = "Try to compile";
  std::cout << str << '\n';
  return 0;
}
