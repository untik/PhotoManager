#include "ImageProcessor.h"
#include "ImageProcessorWorker.h"

ImageProcessor::ImageProcessor(QObject* parent)
	: QObject(parent)
{
	worker = new ImageProcessorWorker(this);
	connect(worker, &ImageProcessorWorker::imageLoaded, this, &ImageProcessor::imageLoaded, Qt::QueuedConnection);
}

ImageProcessor::~ImageProcessor()
{}

void ImageProcessor::loadImage(const QString& fileName)
{
	if (fileName.isEmpty())
		return;
	worker->appendTask(ImageProcessorWorker::TaskLoadImage, 0, fileName, true);
}

void ImageProcessor::preloadImage(const QString& fileName)
{
	if (fileName.isEmpty())
		return;
	worker->appendTask(ImageProcessorWorker::TaskPreloadImage, 0, fileName, false);
}
