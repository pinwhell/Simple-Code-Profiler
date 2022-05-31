#include "TimeProfiler.h"
#include "FPSTimer.h"
#include "Utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#define GetTID() GetCurrentThreadId()
#elif __linux__ || ANDROID

#endif

TimeProfiler* gpTimeProfiler = nullptr;

TimeProfiler::TimeProfiler()
{}

TimeProfiler::~TimeProfiler()
{
    LogToConsole();
    LogToFile();
}

void TimeProfiler::LogToFile()
{
    std::string fileName = "profile-time-log-" + getTimeStr() + ".txt";
    std::ofstream lFile(fileName);

    if (lFile.is_open())
        LogToStream(lFile);
}

void TimeProfiler::StoreAverages(std::vector<std::pair<std::string, float>>& outAvgs)
{
    for (const auto& currKvObj : m_AveragesDurations)
        outAvgs.push_back(currKvObj);
}

void TimeProfiler::LogToConsole()
{
    LogToStream(std::cout);
}

void TimeProfiler::LogToStream(std::ostream& outStream)
{
    std::lock_guard<std::mutex> logMtxGuard(m_LogMtx);

    // Logging the Times Durations Averages
    std::vector<std::pair<std::string, float>> lAverages;

    StoreUMap(m_AveragesDurations, lAverages);

    std::sort(lAverages.begin(), lAverages.end(), [](const std::pair<std::string, float>& p1, const std::pair<std::string, float>& p2) {
        return p1.second > p2.second;
        });


    outStream << std::setw(20) << std::left << "Nombre" << std::setw(13) << std::left << "Average" << std::setw(10) << std::left << "Unit" << std::endl;

    for (const auto& lAverage : lAverages)
    {
        float millisecsRunning = lAverage.second;
        char* pUnitsStr = (char*)"ms";
        float lUnits = millisecsRunning;

        if (lUnits >= 1000)
        {
            pUnitsStr = (char*)"sec";
            lUnits /= 1000.f;

            if (lUnits >= 60)
            {
                pUnitsStr = (char*)"min";
                lUnits /= 60;

                if (lUnits >= 60)
                {
                    pUnitsStr = (char*)"hour";
                    lUnits /= 60;
                }
            }
        }

        outStream << std::setw(20) << std::left << lAverage.first << std::setw(13) << std::left << lUnits << std::setw(10) << std::left << pUnitsStr << std::endl;
    }

    outStream << std::endl;

    // Logging last functions in threads
    std::vector<std::pair<uintptr_t, std::string>> lLastFuncsByThread;

    StoreUMap(m_LastFunctionsByThread, lLastFuncsByThread);

    outStream << std::setw(20) << std::left << "Thread ID" << std::setw(13) << std::left << "Function" << std::endl;

    for(const auto& lFuncInThread : lLastFuncsByThread)
        outStream << std::setw(20) << std::left << lFuncInThread.first << std::setw(13) << std::left << lFuncInThread.second << std::endl;

    outStream << std::endl;
}

void TimeProfiler::Reset()
{

}

ProfileProcInfo TimeProfiler::Start(const std::string& crFuncSignatureStr)
{
    std::lock_guard<std::mutex> startMtxGuard(m_StartMtx);

    m_LastFunctionsByThread[GetTID()] = crFuncSignatureStr;

    if (!UMapExist(m_StartEpochs, crFuncSignatureStr))
        m_StartEpochs[crFuncSignatureStr] = std::stack<double>();

    m_StartEpochs[crFuncSignatureStr].push(FPSTimer::GetCurrMillis());

    return ProfileProcInfo(this, crFuncSignatureStr);
}

void TimeProfiler::End(const std::string& crFuncSignatureStr)
{
    std::lock_guard<std::mutex> endMtxGuard(m_EndMtx);
    double lTimeDiff = 0.0;
    auto& rEpochs = m_StartEpochs[crFuncSignatureStr];

    if (!UMapExist(m_StartEpochs, crFuncSignatureStr) || rEpochs.empty())
        __debugbreak();

    lTimeDiff = FPSTimer::GetCurrMillis() - rEpochs.top(); rEpochs.pop();

    if (!UMapExist(m_AveragesDurations, crFuncSignatureStr))
        m_AveragesDurations[crFuncSignatureStr] = lTimeDiff;
    else
    {
        m_AveragesDurations[crFuncSignatureStr] += lTimeDiff;
        m_AveragesDurations[crFuncSignatureStr] /= 2.f;
    }
}

ProfileProcInfo::ProfileProcInfo(TimeProfiler* pTimeProfiler, const std::string& rProcSignature)
    : m_pTimeProfiler(pTimeProfiler)
    , m_ProcSignature(rProcSignature)
{}

ProfileProcInfo::~ProfileProcInfo()
{
    m_pTimeProfiler->End(m_ProcSignature);
}
