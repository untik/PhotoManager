#include "Settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

Settings::Settings()
{}

Settings::Settings(const QString& fileName)
{
	load(fileName);
}

Settings::~Settings()
{}

bool Settings::load(const QString& fileName)
{
	loadedFileName = fileName;

	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
		return false;

	QByteArray jsonData = file.readAll();
	QJsonDocument document = QJsonDocument::fromJson(jsonData);

	QJsonObject rootObject = document.object();
	values = rootObject.toVariantHash();
	return true;
}

bool Settings::save()
{
	if (loadedFileName.isEmpty())
		return false;

	QJsonDocument document(QJsonObject::fromVariantHash(values));
	QByteArray jsonData = document.toJson(QJsonDocument::JsonFormat::Indented);

	QFile file(loadedFileName);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return false;

	file.write(jsonData);
	return true;
}

QVariant& Settings::value(const QString& key)
{
	return values[key];
}

void Settings::initializeValue(const QString& key, QVariant value)
{
	if (!values.contains(key))
		values[key] = value;
}
