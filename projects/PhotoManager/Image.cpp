#include "Image.h"
#include <QImageReader>
#include <QFileInfo>
#include <QElapsedTimer>
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

	QImageReader imageReader(imageFilePath);
	imageReader.setAutoTransform(true);
	imageData = imageReader.read().convertToFormat(QImage::Format_ARGB32_Premultiplied);

	qint64 imageElapsed = timer.restart();

	MetadataReader metadataReader;
	imageMetadata = metadataReader.load(imageFilePath);

	qint64 metadataElapsed = timer.restart();

	QImageIOHandler::Transformations t = imageReader.transformation();
	qDebug() << "Loaded in:" << imageElapsed + metadataElapsed << "ms, cost:" << cacheSize() << "MB, transformations:" << t << ", metadata:" << metadataElapsed << "ms";
}

int Image::cacheSize() const
{
	return qMax(1, static_cast<int>((double)imageData.byteCount() / (1024 * 1024) + 0.5));
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
