#include "signal_timer.h"
#include <QBasicTimer>

using namespace om;

struct SignalTimer::Impl
{
    QBasicTimer               timer;
    std::function<void(void)> emit_function;
    QObject*                  target           = nullptr;
    unsigned int              delay            = QUEUED_CALL;
    unsigned int              interval         = 0;
    unsigned int              current_interval = 0;
};

SignalTimer::SignalTimer() : d(std::make_unique<Impl>())
{}

SignalTimer::SignalTimer(QObject* target, std::function<void(void)> emit_function) : SignalTimer()
{
    Initialize(target, std::move(emit_function));
}

SignalTimer::~SignalTimer()
{}

bool SignalTimer::IsInitialized() const
{
    return d->target && d->emit_function;
}

void SignalTimer::Initialize(QObject* target, std::function<void(void)> emit_function)
{
    d->target        = target;
    d->emit_function = std::move(emit_function);
}

unsigned int SignalTimer::GetDelay() const
{
    return d->delay;
}

void SignalTimer::SetDelay(unsigned int val)
{
    d->delay = val;
}

unsigned int SignalTimer::GetInterval() const
{
    return d->interval;
}

void SignalTimer::SetInterval(unsigned int val)
{
    d->interval = val;
}

int SignalTimer::GetTimerId() const
{
    return d->timer.timerId();
}

bool SignalTimer::IsActive() const
{
    return d->timer.isActive();
}

void SignalTimer::Start(int interval)
{
    d->current_interval = interval;
    d->timer.start(d->current_interval, d->target);
}

void SignalTimer::Start()  // Use this method when you needs to enqueue your signal call
{
    if (IsActive() || !IsInitialized())
        return;

    if (d->delay == NO_DELAY)
    {
        if (d->interval)
            Start(d->interval);
        d->emit_function();
    }

    if (d->delay == QUEUED_CALL)
        Start(0);
    else if (d->delay == EQUALS_TO_INTERVAL)
        Start(d->interval);
    else
        Start(d->delay);
}

void SignalTimer::Stop()
{
    d->timer.stop();
}

void SignalTimer::OnTimerEvent()  // Call this method on this timers timerEvent
{
    d->emit_function();
    if (d->interval != d->current_interval)
        Start(d->interval);
}
