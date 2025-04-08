#pragma once

#include <QString>

struct MetadataItem
{
	QString label;
	QString stringValue;
	QString unit;
	int rawIntegerValue = 0;
	QByteArray exifKey;

	MetadataItem() {}
	MetadataItem(const QString& itemLabel) : label(itemLabel)  {}
	MetadataItem(const QString& itemLabel, const QByteArray& itemExifKey) : label(itemLabel), exifKey(itemExifKey) {}
};
