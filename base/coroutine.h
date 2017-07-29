#pragma once

#include <cstdint>
#include <cassert>
#include <ctime>
#include <functional>
#include <string>
#include <list>
#include <atomic>
#include <thread>

#include <Windows.h>
#define sleep(ms) Sleep(ms)

#include <QDebug>
#define _dbg() (qDebug() << cr::uptime())
typedef LPVOID ptr_t;

namespace cr {

extern std::atomic<bool> closed;

struct routine_t
{
    std::function<void()> func;
    bool once = true;
    bool finished = false;
    ptr_t fiber = nullptr;

    routine_t(std::function<void()> func)
        : func(func) {}
    ~routine_t() { ::DeleteFiber(fiber); }
};

typedef routine_t* routine_ptr;

struct ordinator_t
{
    std::list<routine_ptr> routines;
    routine_ptr current = nullptr;
    ptr_t fiber = nullptr;

    ~ordinator_t() {
        for (auto routine : routines)
            delete routine;
    }
};

thread_local extern ordinator_t ordinator;

inline void switchto(routine_ptr routine)
{
    assert(ordinator.current != routine);
    ordinator.current = routine;
    ::SwitchToFiber(routine == (routine_ptr)0 ?
                    ordinator.fiber : routine->fiber);
}

inline void yield()
{
    assert(ordinator.current != (routine_ptr)0);
    switchto((routine_ptr)0);
}

inline clock_t uptime()
{
    clock_t now = clock();
    return now * 1000 / CLOCKS_PER_SEC;
}

inline void delay(int msecs)
{
    clock_t until = uptime() + msecs;
    cr::yield();  // yield even if msecs <= 0
    while (until > uptime())
        cr::yield();
}

inline routine_ptr loop(std::function<void()> func)
{
    routine_t *routine = new routine_t(func);
    routine->once = false;
    ordinator.routines.push_back(routine);
    // qDebug() << &ordinator << ordinator.routines.size();
    return routine;
}

inline routine_ptr once(std::function<void()> func)
{
    routine_t *routine = new routine_t(func);
    ordinator.routines.push_back(routine);
    return routine;
}

inline void destroy(routine_ptr routine)
{
    assert(routine != (routine_ptr)0);
    ordinator.routines.remove(routine);
    delete routine;
}

inline void __stdcall entry(ptr_t lpParameter)
{
    routine_ptr routine = (routine_ptr)lpParameter;
    assert(routine != (routine_ptr)0);

    do {
        routine->func();
        cr::yield();
    } while (!routine->once);

    routine->finished = true;
    cr::yield();
}

inline int resume(routine_ptr routine)
{
    assert(ordinator.current == (routine_ptr)0);

    if (routine->finished)
        return -1;

    if (routine->fiber == nullptr)
        routine->fiber = ::CreateFiber(0, entry, (ptr_t)routine);

    switchto(routine);
    return 0;
}

inline routine_ptr current()
{
    return ordinator.current;
}

inline void exec(int rest)
{
    assert(ordinator.current == nullptr);
    assert(ordinator.fiber == nullptr);

    auto& routines = ordinator.routines;
    auto iter = routines.begin();
    assert(iter != routines.end());

    ordinator.fiber = ::ConvertThreadToFiber(nullptr);
    _dbg() << "total routines:" << &routines << routines.size();

    do {
        iter = resume(*iter) < 0 ?
               routines.erase(iter) : ++iter;

        // Re-scan routines
        if (iter == routines.end())
            iter = routines.begin();

        if (rest > 0) sleep(rest);
    } while (routines.size() > 0 && !closed);

    iter = routines.begin();
    while (iter != routines.end())
        iter = routines.erase(iter);

    closed.store(true);
}

} //namespace cr
