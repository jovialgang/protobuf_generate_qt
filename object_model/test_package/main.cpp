#include "test_object.h"

#include <gui/object_model/object_model.h>
#include <gui/object_model/object_model_map_wrapper.h>
#include <gui/object_model/object_model_vector_wrapper.h>

#include <QGuiApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char* argv[])
{
    QGuiApplication a(argc, argv);

    QCoreApplication::setOrganizationName("NPP NTT ROOM 11");
    QCoreApplication::setApplicationName("Object model");

    om::ObjectModel m(TestObject::staticMetaObject);
    // QObject::connect(&m, &ObjectModel::itemDataChanged, [](const RoleIds& roles) { qDebug() << "ObjectModel::dataChanged: " << roles; });
    // QObject::connect(&m, &ObjectModel::dataChanged, [](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
    //    qDebug() << "QAbstractItemModel::dataChanged: " << topLeft << bottomRight << roles;
    //});

    m.Append();
    m.Append(new TestObject(1, "a"));
    m.Set(0, new TestObject(2, "b"));
    m.Insert(0, new TestObject(3, "c"));
    m.Remove(1);

    auto objects = QVector<QObject*>{ new TestObject(4, "d"), new TestObject(5, "e"), new TestObject(6, "f") };
    m.SetItems(objects);

    om::ObjectModelMapWrapper<QString, TestObject> map_wrapper(&m, "name");

    auto* object = new TestObject(7, "g");
    map_wrapper.Insert(QString::number(object->GetId()), object);
    map_wrapper.Clear();

    om::ObjectModelVectorWrapper<TestObject> vector_wrapper(&m);

    vector_wrapper.Append(new TestObject(7, "g"));
    vector_wrapper.Clear();

#ifndef OBJECT_MODEL_LOCAL_TESTS_ENABLED
    QTimer::singleShot(1000, &a, SLOT(quit()));
#endif
    a.exec();

    return 0;
}
