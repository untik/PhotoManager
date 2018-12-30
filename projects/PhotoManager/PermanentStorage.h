#pragma once

#include <QString>
#include <QVector>

class PermanentStorage
{
public:
	PermanentStorage();
	~PermanentStorage();

	void setListFileName(const QString& fileName);

	bool toggleMarkedState(const QString& fileName);
	void setMarkedState(const QString& fileName, bool isMarked);
	bool isMarked(const QString& fileName) const;

	void save();
	void load();

private:
	QString listFile;
	QVector<QString> markedFiles;
};
