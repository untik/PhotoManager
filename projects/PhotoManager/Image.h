#pragma once

#include <QImage>
#include "MetadataCollection.h"

class Image
{
public:
	Image();
	~Image();

	void load(const QString& fileName);
	int cacheSize() const;

	QSize size() const;

	void rotate(double angle);

	const QImage& image() const { return imageData; }
	const QString& absoluteFilePath() const { return imageFilePath; }
	const QString& fileName() const { return imageFileName; }
	const MetadataCollection& metadata() const { return imageMetadata; }

private:
	QImage imageData;
	QString imageFilePath;
	QString imageFileName;
	MetadataCollection imageMetadata;
};

Q_DECLARE_METATYPE(Image);
