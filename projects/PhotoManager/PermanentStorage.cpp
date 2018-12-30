#include "PermanentStorage.h"
#include <QFile>
#include <QFileInfo>

PermanentStorage::PermanentStorage()
{}

PermanentStorage::~PermanentStorage()
{}

void PermanentStorage::setListFileName(const QString& fileName)
{
	listFile = fileName;
	load();
}

bool PermanentStorage::toggleMarkedState(const QString& fileName)
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

void PermanentStorage::setMarkedState(const QString& fileName, bool isMarked)
{
	if (isMarked) {
		if (!markedFiles.contains(fileName))
			markedFiles.append(fileName);
	} else {
		markedFiles.removeOne(fileName);
	}
	save();
}

bool PermanentStorage::isMarked(const QString& fileName) const
{
	return markedFiles.contains(fileName);
}

void PermanentStorage::save()
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
		}
		file.write(text.toUtf8());
	}
}

void PermanentStorage::load()
{
	QFile file(listFile);
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		QString text = QString::fromUtf8(file.readAll());
		QStringList lines = text.split('\n');
		QFileInfo fileInfo(listFile);
		for (int i = 0; i < lines.count(); i++) {
			QString lineText = lines.at(i).trimmed();
			if (!lineText.isEmpty())
				markedFiles.append(fileInfo.absolutePath() + "/" + lineText);
		}
		file.write(text.toUtf8());
	}
}
