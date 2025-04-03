#pragma once

#include <QMetaObject>
#include <functional>
#include <limits>
#include <memory>

namespace om
{
class SignalTimer
{
    Q_GADGET
    Q_PROPERTY(unsigned int delay READ GetDelay WRITE SetDelay FINAL)
    Q_PROPERTY(unsigned int interval READ GetInterval WRITE SetInterval FINAL)
public:
    enum DelayType : unsigned int {
        NO_DELAY           = 0,                                        // Start() will emit signal immediately and starts timer with duration equals to interval_ if it is > 0
        QUEUED_CALL        = 1,                                        // starts timer with duration equals to 0 (equals to QMetaObject::invokeMethod with Qt::QueuedConnection)
        EQUALS_TO_INTERVAL = std::numeric_limits<unsigned int>::max()  // starts timer with duration equals to interval_
    };
    Q_ENUM(DelayType)

    SignalTimer();
    SignalTimer(QObject* target, std::function<void(void)> emit_function);
    ~SignalTimer();

    bool IsInitialized() const;
    void Initialize(QObject* target, std::function<void(void)> emit_function);

    unsigned int GetDelay() const;
    void         SetDelay(unsigned int val);  // DelayType or  any int

    unsigned int GetInterval() const;
    void         SetInterval(unsigned int val);

    int  GetTimerId() const;
    bool IsActive() const;

    void Start();  // Use this method when you needs to enqueue your signal call
    void Stop();
    void OnTimerEvent();  // Call this method on this timers timerEvent

private:
    void Start(int interval);

    struct Impl;
    std::unique_ptr<Impl> d;
};
}  // namespace om
