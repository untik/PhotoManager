#pragma once

#include <QThread>
#include <QVariant>
#include <QVector>
#include <QCache>
#include "Image.h"

class QMutex;
class QWaitCondition;

class ImageProcessorWorker : public QThread
{
	Q_OBJECT

public:
	enum TaskType
	{
		TaskLoadImage,
		TaskPreloadImage,
	};

	struct TaskData
	{
		int id;
		TaskType type;
		QVariant data;

		TaskData() : id(0) {}
	};

	ImageProcessorWorker(QObject* parent = nullptr);
	~ImageProcessorWorker();

	void appendTask(TaskType type, int id, const QVariant& data, bool removePrevious);

signals:
	void imageLoaded(const Image& image);

protected:
	void run();
	void processTask(const TaskData& task);

private:
	void taskLoadImage(const QString& fileName);
	void taskPreloadImage(const QString& fileName);

private:
	QMutex* mutex;
	QWaitCondition* waitCondition;
	QVector<TaskData> tasks;
	QCache<QString, Image> cache;
};

Q_DECLARE_METATYPE(ImageProcessorWorker::TaskData);
Q_DECLARE_TYPEINFO(ImageProcessorWorker::TaskData, Q_MOVABLE_TYPE);
