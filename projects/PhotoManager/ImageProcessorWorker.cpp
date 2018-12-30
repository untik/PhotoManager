#include "ImageProcessorWorker.h"
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>

ImageProcessorWorker::ImageProcessorWorker(QObject* parent)
	: QThread(parent), cache(1024)
{
	qRegisterMetaType<TaskData>();
	qRegisterMetaType<Image>();

	mutex = new QMutex();
	waitCondition = new QWaitCondition();
	start();
}

ImageProcessorWorker::~ImageProcessorWorker()
{
	requestInterruption();
	waitCondition->wakeAll();
	wait();
	delete mutex;
	delete waitCondition;
}

void ImageProcessorWorker::appendTask(TaskType type, int id, const QVariant& data, bool removePrevious)
{
	mutex->lock();
	if (removePrevious) {
		for (int i = 0; i < tasks.count(); i++) {
			if (tasks.at(i).id == id) {
				tasks.removeAt(i);
				i--;
			}
		}
	}
	TaskData task;
	task.id = id;
	task.type = type;
	task.data = data;
	
	tasks.append(task);
	mutex->unlock();
	waitCondition->wakeAll();
}

void ImageProcessorWorker::run()
{
	while (!isInterruptionRequested()) {
		mutex->lock();
		if (tasks.isEmpty()) {
			// Wait for more requests
			waitCondition->wait(mutex);
			mutex->unlock();
		} else {
			TaskData task = tasks.takeFirst();
			mutex->unlock();
			processTask(task);
		}
	}
}

void ImageProcessorWorker::processTask(const TaskData& task)
{
	switch (task.type) {
		case TaskLoadImage:
			taskLoadImage(task.data.toString());
			break;
		case TaskPreloadImage:
			taskPreloadImage(task.data.toString());
			break;
	}
}

void ImageProcessorWorker::taskLoadImage(const QString& fileName)
{
	if (cache.contains(fileName)) {
		Image* image = cache.object(fileName);
		emit imageLoaded(*image);
	} else {
		Image* image = new Image();
		image->load(fileName);
		cache.insert(image->absoluteFilePath(), image, image->cacheSize());
		emit imageLoaded(*image);
	}
}

void ImageProcessorWorker::taskPreloadImage(const QString& fileName)
{
	if (cache.contains(fileName))
		return;

	Image* image = new Image();
	image->load(fileName);
	cache.insert(image->absoluteFilePath(), image, image->cacheSize());
}
