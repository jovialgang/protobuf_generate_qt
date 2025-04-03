#pragma once
#include <QDebug>
#include <QObject>
#include <QString>

class TestSuperObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ GetId WRITE SetId NOTIFY idChanged)
public:
    Q_INVOKABLE TestSuperObject(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~TestSuperObject() {}

    int  GetId() const { return id_; }
    void SetId(int val)
    {
        id_ = val;
        emit idChanged();
    }

signals:
    void idChanged();

protected:
    int id_ = -1;
};

class TestPropertyObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double x READ GetX WRITE SetX NOTIFY xChanged)
    Q_PROPERTY(float y READ GetY WRITE SetY NOTIFY yChanged)
public:
    Q_INVOKABLE TestPropertyObject(QObject* parent = nullptr) : QObject(parent) {}
    ~TestPropertyObject() override {}

    double GetX() const { return x_; }
    void   SetX(double x)
    {
        x_ = x;
        emit xChanged();
    }

    float GetY() const { return y_; }
    void  SetY(float y)
    {
        y_ = y;
        emit yChanged();
    }

signals:
    void xChanged();
    void yChanged();

private:
    double x_;
    float  y_;
};

class TestObject : public TestSuperObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ GetName WRITE SetName NOTIFY nameChanged)
    Q_PROPERTY(TestPropertyObject* object READ GetObject WRITE SetObject NOTIFY objectChanged)
public:
    Q_INVOKABLE TestObject(QObject* parent = nullptr) : TestSuperObject(parent) {}
    TestObject(int id, const QString& name, QObject* parent = nullptr) : TestSuperObject(parent), name_(name) { id_ = id; }
    ~TestObject() override { qDebug() << QString("TestObject id = %1 destroyed").arg(id_); }

    QString GetName() const { return name_; }
    void    SetName(const QString& val)
    {
        name_ = val;
        emit nameChanged();
    }

    TestPropertyObject* GetObject() const { return object_; }
    void                SetObject(TestPropertyObject* object)
    {
        object_ = object;
        emit objectChanged();
    }

signals:
    void nameChanged();
    void objectChanged();

private:
    QString             name_;
    TestPropertyObject* object_ = nullptr;
};
