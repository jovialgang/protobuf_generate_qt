#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QScopeGuard>
#include <QQmlContext>

#include <gui/object_model/object_model_qml.h>
#include <test_qt_pb.h>
#include <QQuickStyle>
#include <QFont>

#include "filesystemqml.h"

int main(int argc, char* argv[])
{
    qDebug() << ("----- QML utilities trivial test ----");
    auto log_exit = qScopeGuard([&] { qDebug() << ("Application exited"); });

    QGuiApplication application(argc, argv);
    QQuickStyle::setStyle("Material");

    om::RegisterQmlTypes();

    fsysqmlutils::Initialize();
    fsysqmlutils::QmlFileProxy qml_filesystem;

    test_qt_pb::Initialize();

    protogeneratorqt::TestObject test_object;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("testData", QVariant::fromValue(&test_object));
    engine.rootContext()->setContextProperty("qmlFileSystem", QVariant::fromValue(&qml_filesystem));

    engine.load(QUrl("qrc:/qml/TestWindow.qml"));

    qDebug() << ("Starting application event loop...");
    return application.exec();
}

