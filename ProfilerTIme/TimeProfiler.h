#pragma once

#include <unordered_map>
#include <mutex>
#include <stack>

#define PROFILE_TIME
#define MAIN_THREAD_REPORT_CHECK

class TimeProfiler;

struct ProfileProcInfo {
private:
    TimeProfiler* m_pTimeProfiler;
    std::string m_ProcSignature;
public:

    ProfileProcInfo(TimeProfiler* pTimeProfiler, const std::string& rProcSignature);

    ~ProfileProcInfo();
};

class TimeProfiler {

private:
    std::mutex m_StartMtx;
    std::mutex m_EndMtx;
    std::mutex m_LogMtx;
    std::mutex m_MainThreadEpochMtx;
    double m_LastMainThreadEpoch;
    std::unordered_map<std::string, std::stack<double>> m_StartEpochs;
    std::unordered_map<std::string, float> m_AveragesDurations;
    std::unordered_map<uintptr_t, std::string> m_LastFunctionsByThread;

public:
    TimeProfiler();

    ~TimeProfiler();

    void LogToFile();

    void StoreAverages(std::vector<std::pair<std::string, float>>& outAvgs);

    void LogToConsole();

    void LogToStream(std::ostream& outStream);

    void Reset();

    ProfileProcInfo Start(const std::string& crFuncSignatureStr);

    void End(const std::string& crFuncSignatureStr);

    void LogExit();

    void MainThreadReport();

    std::mutex& getMainThreadEpochMtx();
};

extern TimeProfiler* gpTimeProfiler;

#ifdef PROFILE_TIME
#define PROF_CALL(x)\
{ \
    auto profStruct = gpTimeProfiler->Start(#x); \
    x; \
}

#define PROF_THREAD(x) auto profStruct = gpTimeProfiler->Start(#x);

#else
#define PROF_CALL(x) x

#define PROF_THREAD(x) do {} while(false)
#endif