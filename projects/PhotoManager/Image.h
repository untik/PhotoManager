#pragma once

#include <QImage>
#include <qimageiohandler.h>
#include "MetadataCollection.h"

class Image
{
public:
	enum class Type
	{
		Bitmap, Vector, Movie
	};

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
	Type type() const { return imageType; }

private:
	QImageIOHandler::Transformations loadImageFromFile(const QString& fileName, const QString& fileExtension);

private:
	QImage imageData;
	QString imageFilePath;
	QString imageFileName;
	MetadataCollection imageMetadata;
	Type imageType = Type::Bitmap;
};

Q_DECLARE_METATYPE(Image);
