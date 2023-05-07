#include <iostream>
#include <Windows.h>
#include <functional>
#include <thread>
#include "TimeProfiler.h"

void SomeFunc2()
{
    Sleep(100);
}

void SomeFunc3()
{
    Sleep(150);
}
void SomeFunc4()
{
    Sleep(300);
}

void SomeFunc1() // 1360 ms
{
    // Do Some Work

    PROF_CALL(SomeFunc2()); // 100 ms

    PROF_CALL(SomeFunc3()); // 150 ms

    PROF_CALL(SomeFunc4()); // 100 - 300 ms
    
    PROF_CALL(Sleep(810));
}

/*

-(100 + 150 + 300 - 1360) = -(-x)

*/

template<typename T>
T getRandom(T min, T max)
{
    return (T)(rand() % (max - min + 1) + min);
}

void SleepRandom()
{
    Sleep(getRandom<DWORD>(20, 2000));
}

int main()
{
    TimeProfiler lTimeProfiler; gpTimeProfiler = &lTimeProfiler;
    srand(GetCurrentProcessId() * time(NULL));

    PROF_CALL(SomeFunc1());
    std::thread tFunc1([] {
        PROF_THREAD(tFunc1);

        for (int i = 0; i < 3; i++)
        {
            PROF_CALL(SomeFunc2());
            SleepRandom();
        }
    });

    std::thread tFunc2([] {
        PROF_THREAD(tFunc2);

        for (int i = 0; i < 5; i++)
        {
            PROF_CALL(SomeFunc2());
            SleepRandom();
        }
    });

    tFunc1.join();
    tFunc2.join();

    while(true)
    {
    	Sleep(100);
    	gpTimeProfiler->MainThreadReport();
    }
}

/*
Nombre                          Tiempo
void SomeFunc1()                1.36 sec  
void SomeFunc2()                100 ms
void SomeFunc3()                150 ms
void SomeFunc4()                300 ms
*/