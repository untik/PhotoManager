#pragma once

#include <QString>
#include <QVector>
#include "MarkerType.h"

class MarkerFile
{
public:
	MarkerFile(const QString& baseFileName, MarkerType markerFileType);
	~MarkerFile();

	void loadDirectory(const QString& directory);
	void load(const QString& fileName);
	void load();

	void save();

	bool toggleMarkedState(const QString& fileName);
	void setMarkedState(const QString& fileName, bool isMarked);
	bool isMarked(const QString& fileName) const;

	const QString& baseName() const { return markerBaseName; }
	MarkerType type() const { return markerType; }

private:
	QString listFile;
	QVector<QString> markedFiles;
	QString markerBaseName;
	MarkerType markerType;
};

