#pragma once
#include <QDebug>
#include <QObject>
#include <QString>

class TestSuperObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ GetId WRITE SetId NOTIFY idChanged)
public:
    TestSuperObject(QObject* parent = nullptr) : QObject(parent) {}

    int  GetId() const { return id_; }
    void SetId(int val)
    {
        id_ = val;
        emit idChanged();
    }

    void EmitChanged() { emit changed(); }

signals:
    void idChanged();
    void changed();

protected:
    int id_ = -1;
};

class CoordTypeObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Type type READ GetType WRITE SetType NOTIFY typeChanged)
public:
    enum class Type { Type0, Type1, Type2, Type3 };
    Q_ENUM(Type);

    CoordTypeObject(QObject* parent = nullptr) : QObject(parent) {}
    CoordTypeObject(Type type, QObject* parent = nullptr) : QObject(parent), type_(type) {}

    Type GetType() const { return type_; }
    void SetType(Type val)
    {
        if (type_ == val)
            return;
        type_ = val;
        emit typeChanged();
    }

    void EmitChanged() { emit changed(); }

signals:
    void typeChanged();
    void changed();

private:
    Type type_ = Type::Type1;
};

class CoordObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double x READ GetX WRITE SetX NOTIFY xChanged)
    Q_PROPERTY(float y READ GetY WRITE SetY NOTIFY yChanged)
    Q_PROPERTY(CoordTypeObject* type READ GetType WRITE SetType NOTIFY typeChanged)
public:
    CoordObject(QObject* parent = nullptr) : QObject(parent) {}
    CoordObject(double x, float y, QObject* parent = nullptr) : QObject(parent), x_(x), y_(y) {}

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

    CoordTypeObject* GetType() const { return type_; }
    void             SetType(CoordTypeObject* val)
    {
        if (type_ == val)
            return;
        type_ = val;
        emit typeChanged();
    }

    bool operator==(const CoordObject& r) { return x_ == r.x_ && r.y_ == r.y_; }

signals:
    void xChanged();
    void yChanged();
    void typeChanged();

private:
    double           x_    = 0;
    float            y_    = 0;
    CoordTypeObject* type_ = nullptr;
};

class TestObject : public TestSuperObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ GetName WRITE SetName NOTIFY nameChanged)
    Q_PROPERTY(CoordObject* coord READ GetCoord WRITE SetCoord NOTIFY coordChanged)
public:
    Q_INVOKABLE TestObject(QObject* parent = nullptr) : TestSuperObject(parent) {}
    TestObject(int id, const QString& name, QObject* parent = nullptr) : TestSuperObject(parent), name_(name) { id_ = id; }

    QString GetName() const { return name_; }
    void    SetName(const QString& val)
    {
        name_ = val;
        emit nameChanged();
    }

    CoordObject* GetCoord() const { return coord_; }
    void         SetCoord(CoordObject* coord)
    {
        coord_ = coord;
        emit coordChanged();
    }

    bool operator==(const TestObject& r) { return id_ == r.id_ && name_ == r.name_ && coord_ == r.coord_; }

signals:
    void nameChanged();
    void coordChanged();

private:
    QString      name_;
    CoordObject* coord_ = nullptr;
};
