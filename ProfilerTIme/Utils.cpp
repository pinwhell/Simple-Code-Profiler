#include "Utils.h"
#include <sstream>
#include <iomanip>

void getTimeRefStr(std::string& rOutTime)
{
    std::ostringstream oss;
    auto t = std::time(nullptr);
    tm _tm;
    localtime_s(&_tm, &t);

    oss << std::put_time(&_tm, "%H-%M-%S");
    rOutTime = oss.str();
}

std::string getTimeStr()
{
    std::string outTime = "";

    getTimeRefStr(outTime);

    return outTime;
}
