#include "filesystemqml.h"

#include <QFileInfo>
#include <QTextStream>
#include <QQmlEngine>
#include <QUrl>

namespace fsysqmlutils  {

QmlFileProxy::QmlFileProxy(QObject* parent /*= nullptr*/)
{}

QmlFileProxy::~QmlFileProxy()
{}

void QmlFileProxy::Save(const QByteArray& data, const QUrl& file_path)
{
	QFile file(file_path.toLocalFile());
	if (!file.open(QIODevice::WriteOnly))
		return;
	file.write(data);
}

QByteArray QmlFileProxy::Load(const QUrl& file_path)
{
	QFile file(file_path.toLocalFile());
	if (!file.open(QIODevice::ReadOnly))
		return QByteArray();
	return file.readAll();
}

void Initialize()
{
	qmlRegisterType<QmlFileProxy>("FSysQmlUtils", 1, 0, "QmlFile");
}

} // namespace fsysqmlutils
