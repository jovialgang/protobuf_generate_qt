#pragma once

#include <QObject>
#include <QFile>
#include <QUrl>
#include <QString>

/*
* fsysqmlutils - file system qml utils
* Utils for work whith filesystem from qml
*/
namespace fsysqmlutils {

class QmlFileProxy : public QFile {
	Q_OBJECT
public: 
	explicit QmlFileProxy(QObject* parent = nullptr);
	~QmlFileProxy();
public slots:
	void Save(const QByteArray& data, const QUrl& file_path);
	QByteArray Load(const QUrl& file_path);
};

void Initialize();

} // namespace fsysqmlutils
