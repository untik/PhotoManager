#pragma once

#include <QString>
#include <QHash>
#include <QVariant>

class Settings
{
public:
	Settings();
	Settings(const QString& fileName);
	~Settings();

	bool load(const QString& fileName);
	bool save();

	QVariant& value(const QString& key);
	void initializeValue(const QString& key, QVariant value);

private:
	QString loadedFileName;
	QHash<QString, QVariant> values;
};

