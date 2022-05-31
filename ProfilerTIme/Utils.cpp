#include "Utils.h"
#include <sstream>
#include <iomanip>

#ifdef _MSC_VER
#define LocalTime(_tm, ptime) localtime_s(_tm, ptime)
#elif __linux__ || ANDROID || __arm__
#define LocalTime(_tm, ptime) *_tm = *localtime(ptime)
#endif

void getTimeRefStr(std::string& rOutTime)
{
    std::ostringstream oss;
    auto t = std::time(nullptr);
    tm _tm;
    LocalTime(&_tm, &t);

    oss << std::put_time(&_tm, "%H-%M-%S");
    rOutTime = oss.str();
}

std::string getTimeStr()
{
    std::string outTime = "";

    getTimeRefStr(outTime);

    return outTime;
}
