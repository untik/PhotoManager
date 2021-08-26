#include "MarkerFile.h"
#include <QFile>
#include <QFileInfo>

MarkerFile::MarkerFile(const QString& baseFileName, MarkerType markerFileType)
	: markerBaseName(baseFileName), markerType(markerFileType)
{}

MarkerFile::~MarkerFile()
{}

void MarkerFile::loadDirectory(const QString& directory)
{
	listFile = directory + "/" + markerBaseName;
	load();
}

void MarkerFile::load(const QString& fileName)
{
	listFile = fileName;
	load();
}

void MarkerFile::load()
{
	QFile file(listFile);
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		QString text = QString::fromUtf8(file.readAll());
		QStringList lines = text.split('\n');
		QFileInfo fileInfo(listFile);
		for (int i = 0; i < lines.count(); i++) {
			QString lineText = lines.at(i).trimmed();
			if (!lineText.isEmpty() && !lineText.endsWith(".*/n"))
				markedFiles.append(fileInfo.absolutePath() + "/" + lineText);
		}
		file.write(text.toUtf8());
	}
}

void MarkerFile::save()
{
	QFile file(listFile);

	if (markedFiles.isEmpty()) {
		if (file.exists())
			file.remove();
		return;
	}

	if (file.open(QFile::WriteOnly | QFile::Truncate)) {
		QString text;
		for (int i = 0; i < markedFiles.count(); i++) {
			QFileInfo fileInfo(markedFiles.at(i));
			text.append(fileInfo.fileName());
			text.append("\n");
			text.append(fileInfo.baseName());
			text.append(".*\n"); // Ignore file extension when selecting
		}
		file.write(text.toUtf8());
	}
}


bool MarkerFile::toggleMarkedState(const QString& fileName)
{
	if (markedFiles.contains(fileName)) {
		markedFiles.removeOne(fileName);
		save();
		return false;
	} else {
		markedFiles.append(fileName);
		save();
		return true;
	}
}

void MarkerFile::setMarkedState(const QString& fileName, bool isMarked)
{
	if (isMarked) {
		if (!markedFiles.contains(fileName))
			markedFiles.append(fileName);
	} else {
		markedFiles.removeOne(fileName);
	}
	save();
}

bool MarkerFile::isMarked(const QString& fileName) const
{
	return markedFiles.contains(fileName);
}
