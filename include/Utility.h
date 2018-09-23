#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <string>

namespace Lib{
namespace Util{

int getRandomNumByLength(int start,int length);
int getRandomNumByRange(int start,int end);

std::vector<std::string> getLocalIps();

}
}

#endif // !UTILITY_H
