#include "Image.h"
#include <QImageReader>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QIcon>
#include <QDebug>
#include "MetadataReader.h"

Image::Image()
{}

Image::~Image()
{}

void Image::load(const QString& fileName)
{
	QElapsedTimer timer;
	timer.start();

	QFileInfo fileInfo(fileName);
	imageFilePath = fileInfo.absoluteFilePath();
	imageFileName = fileInfo.fileName();

	QImageIOHandler::Transformations t = loadImageFromFile(imageFilePath, fileInfo.suffix().toLower());

	qint64 imageElapsed = timer.restart();

	MetadataReader metadataReader;
	imageMetadata = metadataReader.load(imageFilePath);

	qint64 metadataElapsed = timer.restart();

	qDebug() << "Loaded in:" << imageElapsed + metadataElapsed << "ms, cost:" << cacheSize() << "MB, transformations:" << t << ", metadata:" << metadataElapsed << "ms";
}

int Image::cacheSize() const
{
	return qMax(1, static_cast<int>((double)imageData.sizeInBytes() / (1024 * 1024) + 0.5));
}

QSize Image::size() const
{
	return imageData.size();
}

void Image::rotate(double angle)
{
	QTransform transform;
	transform.rotate(angle);

	if (angle == 0 || angle == 90 || angle == 180 || angle == 270)
		imageData = imageData.transformed(transform, Qt::FastTransformation);
	else
		imageData = imageData.transformed(transform, Qt::SmoothTransformation);
}

QImageIOHandler::Transformations Image::loadImageFromFile(const QString& fileName, const QString& fileExtension)
{
	imageType = Type::Bitmap;

	if (fileExtension == "ico") {
		QIcon icon(fileName);
		if (!icon.isNull()) {
			imageData = icon.pixmap(4096).toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
			return QImageIOHandler::TransformationNone;
		}
	}
	else if (fileExtension == "svg") {
		imageType = Type::Vector;
	}
	else if (fileExtension == "gif" || fileExtension == "webp") {
		imageType = Type::Movie;
	}

	// Format Compatibility Notes
	// TGA - Must be without RLE compression and origin must be TopLeft

	QImageReader imageReader(imageFilePath);
	imageReader.setAutoTransform(true);
	imageData = imageReader.read().convertToFormat(QImage::Format_ARGB32_Premultiplied);
	return imageReader.transformation();
}
