#include "TimeProfiler.h"
#include "FPSTimer.h"
#include "Utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <thread>
#include <chrono>

#include <signal.h>

#ifdef _MSC_VER
#include <Windows.h>
#define GetTID() GetCurrentThreadId()
#define __SLEEP(sec) Sleep(sec*1000)
#elif __linux__ || ANDROID || __arm__
#include <pthread.h>
#include <unistd.h>
#define GetTID() pthread_self()
#define __SLEEP(sec) sleep(sec)
#endif

static uint32_t GetCurrMillis() {
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

TimeProfiler* gpTimeProfiler = nullptr;

void SigIntHandler(int signum)
{
    gpTimeProfiler->LogExit();
}

TimeProfiler::TimeProfiler()
    : m_LastMainThreadEpoch(GetCurrMillis())
{

    #ifdef MAIN_THREAD_REPORT_CHECK
    std::thread tProfile([&]{
        for(;;)
        {
            std::lock_guard<std::mutex> mainThreadEpochGuard(m_MainThreadEpochMtx);
            
            if(MILLIS_TO_SEC(GetCurrMillis() - m_LastMainThreadEpoch) > 5)
            {
                LogToConsole();
                exit(2);
            }

            __SLEEP(1);
        }
    });
    #endif

    signal(SIGINT, (sighandler_t)SigIntHandler);

    tProfile.detach();
}

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

    outStream << std::endl;

    outStream << std::setw(45) << std::left << "Nombre" << std::setw(13) << std::left << "Average" << std::setw(10) << std::left << "Unit" << std::endl;

    for (const auto& lAverage : lAverages)
    {
        float millisecsRunning = lAverage.second;
        char* pUnitsStr = (char*)"ms";
        float lUnits = millisecsRunning;

        if (lUnits >= 1000)
        {
            lUnits /= 1000.f;
            pUnitsStr = (char*)"sec";

            if (lUnits >= 60)
            {
                lUnits /= 60.f;
                pUnitsStr = (char*)"min";

                if (lUnits >= 60)
                {
                    lUnits /= 60.f;
                    pUnitsStr = (char*)"hour";

                    if (lUnits >= 24)
                    {
                        lUnits /= 24.f;
                        pUnitsStr = (char*)"days";
                    }
                }
            }
        }

        outStream << std::setw(45) << std::left << lAverage.first << std::setw(13) << std::left << lUnits << std::setw(10) << std::left << pUnitsStr << std::endl;
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

    m_StartEpochs[crFuncSignatureStr].push(GetCurrMillis());

    return ProfileProcInfo(this, crFuncSignatureStr);
}

void TimeProfiler::End(const std::string& crFuncSignatureStr)
{
    std::lock_guard<std::mutex> endMtxGuard(m_EndMtx);
    double lTimeDiff = 0.0;
    auto& rEpochs = m_StartEpochs[crFuncSignatureStr];

    if (!UMapExist(m_StartEpochs, crFuncSignatureStr) || rEpochs.empty())
        gpTimeProfiler->LogExit();

    lTimeDiff = GetCurrMillis() - rEpochs.top(); rEpochs.pop();

    if (!UMapExist(m_AveragesDurations, crFuncSignatureStr))
        m_AveragesDurations[crFuncSignatureStr] = lTimeDiff;
    else
    {
        m_AveragesDurations[crFuncSignatureStr] += lTimeDiff;
        m_AveragesDurations[crFuncSignatureStr] /= 2.f;
    }
}

void TimeProfiler::LogExit()
{
    LogToConsole();
    exit(1);
}

void TimeProfiler::MainThreadReport()
{
    std::lock_guard<std::mutex> mainThreadEpochGuard(m_MainThreadEpochMtx);

    m_LastMainThreadEpoch = GetCurrMillis();
}

std::mutex& TimeProfiler::getMainThreadEpochMtx()
{
    return m_MainThreadEpochMtx;
}

ProfileProcInfo::ProfileProcInfo(TimeProfiler* pTimeProfiler, const std::string& rProcSignature)
    : m_pTimeProfiler(pTimeProfiler)
    , m_ProcSignature(rProcSignature)
{}

ProfileProcInfo::~ProfileProcInfo()
{
    m_pTimeProfiler->End(m_ProcSignature);
}
