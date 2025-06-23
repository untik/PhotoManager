#pragma once

#include <QImage>
#include <QVector>
#include <qimageiohandler.h>
#include "MetadataCollection.h"

class Image
{
public:
	enum class Type
	{
		Bitmap, Vector, Movie
	};

	struct ImageFrame
	{
		QImage image;
		int delay = -1;

		ImageFrame() {}
		ImageFrame(const QImage& img, int delay) : image(img), delay(delay) {}
	};

	Image();
	~Image();

	// Load image data from disk and read metadata
	void load(const QString& fileName);

	// Returns size of loaded image data in MB
	int cacheSize() const;

	// Returns raw image file data
	const QByteArray& data() const { return imageFileData; }

	Type type() const { return imageType; }
	const QString& absoluteFilePath() const { return imageFilePath; }
	const QString& fileName() const { return imageFileName; }
	const MetadataCollection& metadata() const { return imageMetadata; }

	// Rotate current image
	void rotate(double angle);

	// Return current frame
	const QImage& image() const { return currentImage; }
	QSize size() const { return currentImage.size(); }

	// Return number of loaded frames
	int frameCount() const { return imageFrames.count(); }

	// Returns animation delay in ms to the next frame
	int currentFrameDelay();

	// Return current image frame
	int currentFrameIndex() const { return imageCurrentFrameIndex; }

	// Set current image to next image in sequence
	void jumpToNextImage();
	void jumpToPreviousImage();
	
	// Returns info about image load timing
	qint64 elapsedTimeFileLoad() const { return imageTimeFileLoad; }
	qint64 elapsedTimeMetadata() const { return imageTimeMetadata; }
	qint64 elapsedTimeBitmapFrames() const { return imageTimeBitmapFrames; }

private:
	bool loadImageData(const QString& fileName);
	bool readAllFrameDataReader(QIODevice* device);
	bool readAllFrameDataIoHandler(QImageIOHandler* imageHandler);
	bool readAllFrameDataTiff(QIODevice* device);
	bool readAllFrameDataPsd(QIODevice* device);

private:
	QString imageFilePath;
	QString imageFileName;
	QString imageFileType;
	int imageDataSize = 0;
	int imageFrameDataSize = 0;
	QByteArray imageFileData;
	QVector<ImageFrame> imageFrames;
	QImage currentImage;
	int imageCurrentFrameIndex = 0;
	MetadataCollection imageMetadata;
	Type imageType = Type::Bitmap;

	qint64 imageTimeBitmapFrames = 0;
	qint64 imageTimeMetadata = 0;
	qint64 imageTimeFileLoad = 0;
};

Q_DECLARE_METATYPE(Image);
