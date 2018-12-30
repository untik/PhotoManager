#pragma once

#include "MetadataItem.h"
#include <QVector>

class MetadataCollection : public QVector<MetadataItem>
{
public:
	MetadataItem* findKey(const QByteArray& key);
	void removeKey(const QByteArray& key);

	QString toString() const;
};
