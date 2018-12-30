#include "MetadataCollection.h"

QString MetadataCollection::toString() const
{
	QString metadata;
	for (int i = 0; i < count(); i++) {
		const MetadataItem& item = at(i);
		metadata.append(item.label);
		metadata.append(":\n");
		metadata.append(item.stringValue);
		metadata.append("\n\n");
	}
	return metadata;
}

MetadataItem* MetadataCollection::findKey(const QByteArray& key)
{
	MetadataItem* itemData = data();
	for (int i = 0; i < count(); i++) {
		MetadataItem* item = itemData + i;
		if (item->exifKey == key)
			return item;
	}
	return nullptr;
}

void MetadataCollection::removeKey(const QByteArray& key)
{
	for (int i = 0; i < count(); i++) {
		if (at(i).exifKey == key) {
			removeAt(i);
			return;
		}
	}
}
