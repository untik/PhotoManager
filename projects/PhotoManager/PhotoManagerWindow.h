#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PhotoManagerWindow.h"
#include "MarkerType.h"

class ImageViewerWidget;
class ImageProcessor;
class Image;
class PermanentStorage;
class ImageFileList;

class PhotoManagerWindow : public QMainWindow
{
	Q_OBJECT

public:
	PhotoManagerWindow(const QStringList& files, QWidget* parent = nullptr);
	~PhotoManagerWindow();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

private slots:
	void imageLoaded(const Image& image);

private:
	void nextFile(int multiplier = 1);
	void previousFile(int multiplier = 1);
	void toggleMarker(MarkerType marker, bool singleMarker);
	void exportCurrentImage();

private:
	Ui::PhotoManagerWindowClass ui;
	ImageViewerWidget* imageViewer;
	ImageProcessor* imageProcessor;
	ImageFileList* fileList;
};
