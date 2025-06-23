#include "Image.h"
#include <QImageReader>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QBuffer>
#include <QIcon>
#include <QDebug>
#include "MetadataReader.h"
#include "qtiff/qtiffhandler.h"
#include "libqpsd/qpsdhandler.h"

extern void qt_imageTransform(QImage& src, QImageIOHandler::Transformations orient);

Image::Image()
{}

Image::~Image()
{}

void Image::load(const QString& fileName)
{
	imageCurrentFrameIndex = 0;
	imageDataSize = 0;
	imageFrameDataSize = 0;

	loadImageData(fileName);

	// Format Compatibility Notes
	// TGA - Must be without RLE compression and origin must be TopLeft
	// TIFF - Qt doesn't handle jpeg compression, use libtiff directly

	// TODO - PNG keys
	// QStringList QImageReader::textKeys() const
	// QString QImageReader::text(const QString &key) const

	QElapsedTimer timer;
	timer.start();

	QBuffer buffer(&imageFileData);
	buffer.open(QIODevice::ReadOnly);

	// Try all available readers
	bool finishedRead = false;
	if (!finishedRead)
		finishedRead = readAllFrameDataPsd(&buffer);
	if (!finishedRead)
		finishedRead = readAllFrameDataTiff(&buffer);
	if (!finishedRead)
		finishedRead = readAllFrameDataReader(&buffer);

	buffer.close();
	imageFileData.clear();
	imageDataSize = 0;

	imageTimeBitmapFrames = timer.restart();
	qDebug() << "Loaded in:" << imageTimeBitmapFrames << "ms, cost:" << imageFrameDataSize << "MB";

	if (imageFrames.isEmpty())
		return;

	currentImage = imageFrames.at(0).image;

	if (imageFileType == "svg" || imageFileType == "svgz") {
		imageType = Type::Vector;
	} else if (imageFrames.count() > 1) {
		imageType = Type::Movie;
	} else {
		imageType = Type::Bitmap;
	}
}

int Image::cacheSize() const
{
	return qMax(1, imageDataSize + imageFrameDataSize);
}

void Image::rotate(double angle)
{
	QTransform transform;
	transform.rotate(angle);

	if (angle == 0 || angle == 90 || angle == 180 || angle == 270)
		currentImage = currentImage.transformed(transform, Qt::FastTransformation);
	else
		currentImage = currentImage.transformed(transform, Qt::SmoothTransformation);
}

int Image::currentFrameDelay()
{
	if (imageFrames.isEmpty())
		return 0;
	return imageFrames.at(imageCurrentFrameIndex).delay;
}

void Image::jumpToNextImage()
{
	if (imageFrames.isEmpty())
		return;

	imageCurrentFrameIndex++;
	if (imageCurrentFrameIndex >= imageFrames.count())
		imageCurrentFrameIndex = 0;

	currentImage = imageFrames.at(imageCurrentFrameIndex).image;
}

void Image::jumpToPreviousImage()
{
	if (imageFrames.isEmpty())
		return;

	imageCurrentFrameIndex--;
	if (imageCurrentFrameIndex < 0)
		imageCurrentFrameIndex = imageFrames.count() - 1;

	currentImage = imageFrames.at(imageCurrentFrameIndex).image;
}

bool Image::loadImageData(const QString& fileName)
{
	QElapsedTimer timer;
	timer.start();

	QFileInfo fileInfo(fileName);
	imageFilePath = fileInfo.absoluteFilePath();
	imageFileName = fileInfo.fileName();
	imageFileType = fileInfo.suffix().toLower();

	QFile imageFile(imageFilePath);
	if (!imageFile.open(QFile::ReadOnly))
		return false;

	imageFileData = imageFile.readAll();
	imageDataSize = static_cast<int>((double)imageFileData.size() / (1024 * 1024) + 0.5);

	imageTimeFileLoad = timer.restart();

	MetadataReader metadataReader;
	imageMetadata = metadataReader.load(imageFileData, imageFileType);

	imageTimeMetadata = timer.restart();
	qDebug() << "Preloaded in:" << imageTimeFileLoad + imageTimeMetadata << "ms, cost:" << cacheSize() << "MB, metadata:" << imageTimeMetadata << "ms";

	return true;
}

bool Image::readAllFrameDataReader(QIODevice* device)
{
	QImageReader imageReader(imageFilePath);
	imageReader.setAutoTransform(true);

	if (!imageReader.canRead())
		return false;

	// Read all frames
	imageFrameDataSize = 0;
	const int imageCount = imageReader.imageCount();
	for (int i = 0; i < imageCount; i++) {
		ImageFrame frame;
		frame.delay = imageReader.nextImageDelay();
		frame.image = imageReader.read().convertToFormat(QImage::Format_ARGB32_Premultiplied);
		if (frame.image.isNull())
			break;

		int size = static_cast<int>((double)frame.image.sizeInBytes() / (1024 * 1024) + 0.5);
		imageFrameDataSize += size;
		imageFrames.append(frame);

		imageReader.jumpToNextImage();
	}

	return true;
}

bool Image::readAllFrameDataIoHandler(QImageIOHandler* imageHandler)
{
	// Read all frames
	imageFrameDataSize = 0;
	const int imageCount = imageHandler->imageCount();
	for (int i = 0; i < imageCount; i++) {
		ImageFrame frame;
		frame.delay = imageHandler->nextImageDelay();

		QImage img;
		imageHandler->read(&img);
		frame.image = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
		if (frame.image.isNull())
			break;

		QImageIOHandler::Transformations t = imageHandler->option(QImageIOHandler::ImageTransformation).toInt();
		qt_imageTransform(frame.image, t);

		int size = static_cast<int>((double)frame.image.sizeInBytes() / (1024 * 1024) + 0.5);
		imageFrameDataSize += size;
		imageFrames.append(frame);

		imageHandler->jumpToNextImage();
	}

	return true;
}

bool Image::readAllFrameDataTiff(QIODevice* device)
{
	QScopedPointer<QImageIOHandler> tiffHandler(new QTiffHandler());
	tiffHandler->setDevice(device);

	if (!tiffHandler->canRead())
		return false;

	return readAllFrameDataIoHandler(tiffHandler.data());
}

bool Image::readAllFrameDataPsd(QIODevice* device)
{
	QScopedPointer<QImageIOHandler> psdHandler(new QPsdHandler());
	psdHandler->setDevice(device);

	if (!psdHandler->canRead())
		return false;

	return readAllFrameDataIoHandler(psdHandler.data());
}
