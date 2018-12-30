#pragma once

#include <QObject>
#include "Image.h"

class ImageProcessorWorker;


class ImageProcessor : public QObject
{
	Q_OBJECT

public:
	ImageProcessor(QObject* parent = nullptr);
	~ImageProcessor();

	void loadImage(const QString& fileName);
	void preloadImage(const QString& fileName);

signals:
	void imageLoaded(const Image& image);

private:
	ImageProcessorWorker* worker;
};
