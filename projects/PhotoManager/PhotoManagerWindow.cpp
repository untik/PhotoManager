#include "PhotoManagerWindow.h"
#include "ImageViewerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "ImageProcessor.h"
#include "Image.h"
#include "ImageFileList.h"

PhotoManagerWindow::PhotoManagerWindow(const QStringList& files, QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setMargin(0);

	imageViewer = new ImageViewerWidget();
	layout->addWidget(imageViewer);

	ui.centralWidget->setLayout(layout);

	imageProcessor = new ImageProcessor(this);
	connect(imageProcessor, &ImageProcessor::imageLoaded, this, &PhotoManagerWindow::imageLoaded);

	QString fileName;
	if (!files.isEmpty())
		fileName = files.first();
	else
		fileName = "../tools/P1080555.JPG";
	QFileInfo fileInfo(fileName);

	fileList = new ImageFileList();
	fileList->setCurrentFile(fileInfo.absoluteFilePath());

	imageProcessor->loadImage(fileList->fileAtOffset(0).fullFilePath);
	imageProcessor->preloadImage(fileList->fileAtOffset(1).fullFilePath);
	imageProcessor->preloadImage(fileList->fileAtOffset(-1).fullFilePath);

	QString settingsPath(qApp->applicationFilePath());
	settingsPath = QFileInfo(settingsPath).absoluteDir().absoluteFilePath("PhotoManager.json");

	settings.load(settingsPath);
	settings.initializeValue("window.fullscreen", true);
	settings.initializeValue("window.maximized", true);

	Qt::WindowStates windowStates = Qt::WindowActive;
	if (settings.value("window.fullscreen").toBool())
		windowStates = (windowStates | Qt::WindowFullScreen);
	if (settings.value("window.maximized").toBool())
		windowStates = (windowStates | Qt::WindowMaximized);

	setWindowState(windowStates);
	show();
}

PhotoManagerWindow::~PhotoManagerWindow()
{
	delete fileList;
}

void PhotoManagerWindow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
		case Qt::Key_Space:
			imageViewer->zoom(ImageViewerWidget::ZoomQuickToggle);
			break;
		case Qt::Key_Plus:
			imageViewer->zoom(ImageViewerWidget::ZoomIn);
			break;
		case Qt::Key_Minus:
			imageViewer->zoom(ImageViewerWidget::ZoomOut);
			break;
		case Qt::Key_Comma:
			imageViewer->rotate(-90);
			break;
		case Qt::Key_Period:
			imageViewer->rotate(90);
			break;
		case Qt::Key_Left:
			if (event->modifiers().testFlag(Qt::ShiftModifier))
				previousFile(10);
			else
				previousFile();
			break;
		case Qt::Key_Right:
			if (event->modifiers().testFlag(Qt::ShiftModifier))
				nextFile(10);
			else
				nextFile();
			break;
		case Qt::Key_Escape:
			this->close();
			break;
		case Qt::Key_O:
			imageViewer->optimize = !imageViewer->optimize;
			imageViewer->recalculate();
			break;
		case Qt::Key_F:
			setWindowState(windowState() ^ Qt::WindowFullScreen);
			break;
		case Qt::Key_X:
			imageViewer->setMarkerState('X', fileList->toggleCurrentImageMarker(MarkerDelete));
			break;
		case Qt::Key_1:
			toggleMarker(Marker1, event->modifiers().testFlag(Qt::ControlModifier));
			break;
		case Qt::Key_2:
			toggleMarker(Marker2, event->modifiers().testFlag(Qt::ControlModifier));
			break;
		case Qt::Key_3:
			toggleMarker(Marker3, event->modifiers().testFlag(Qt::ControlModifier));
			break;
		case Qt::Key_4:
			toggleMarker(Marker4, event->modifiers().testFlag(Qt::ControlModifier));
			break;
		case Qt::Key_5:
			toggleMarker(Marker5, event->modifiers().testFlag(Qt::ControlModifier));
			break;
		case Qt::Key_0:
			fileList->setFilterMode(ImageFileList::FilterNone);
			imageViewer->setMarkerState(Marker1, false);
			imageViewer->setMarkerState(Marker2, false);
			imageViewer->setMarkerState(Marker3, false);
			imageViewer->setMarkerState(Marker4, false);
			imageViewer->setMarkerState(Marker5, false);
			fileList->setCurrentImageMarkers(QVector<MarkerType>());
			break;
		case Qt::Key_I:
			// Show info bar
			imageViewer->toggleShowImageInformation();
			break;
		case Qt::Key_E:
			// Export current image
			exportCurrentImage();
			break;
		case Qt::Key_Delete:
			// Delete or trash current image
			deleteCurrentImage(event->modifiers().testFlag(Qt::ShiftModifier));
			break;
	}
}

void PhotoManagerWindow::mousePressEvent(QMouseEvent* event)
{
	switch (event->button()) {
		case Qt::MouseButton::BackButton:
			previousFile();
			break;
		case Qt::MouseButton::ForwardButton:
			nextFile();
			break;
	}
}

void PhotoManagerWindow::closeEvent(QCloseEvent* event)
{
	settings.save();
	QApplication::exit();
}

void PhotoManagerWindow::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange) {
		settings.value("window.fullscreen") = this->windowState().testFlag(Qt::WindowFullScreen);
		settings.value("window.maximized") = this->windowState().testFlag(Qt::WindowMaximized);
	}
}

void PhotoManagerWindow::imageLoaded(const Image& image)
{
	fileList->setCurrentFile(image.absoluteFilePath());
	const ImageFileList::Item& item = fileList->fileAtOffset(0);

	imageViewer->setImage(image);
	imageViewer->setImageNumber(fileList->currentIndex() + 1, fileList->unfilteredFileCount());
	imageViewer->setMarkerState(item.markers);
}

void PhotoManagerWindow::nextFile(int multiplier)
{
	int offset = 1 * multiplier;
	imageProcessor->loadImage(fileList->fileAtOffset(offset).fullFilePath);
	imageProcessor->preloadImage(fileList->fileAtOffset(offset + 1).fullFilePath);
	imageProcessor->preloadImage(fileList->fileAtOffset(offset + 2).fullFilePath);
}

void PhotoManagerWindow::previousFile(int multiplier)
{
	int offset = -1 * multiplier;
	imageProcessor->loadImage(fileList->fileAtOffset(offset).fullFilePath);
	imageProcessor->preloadImage(fileList->fileAtOffset(offset - 1).fullFilePath);
	imageProcessor->preloadImage(fileList->fileAtOffset(offset - 2).fullFilePath);
}

void PhotoManagerWindow::toggleMarker(MarkerType marker, bool singleMarker)
{
	if (singleMarker) {
		if (fileList->currentFilterMode() == ImageFileList::FilterNone) {
			fileList->setFilterMode(ImageFileList::FilterSingleMarker, marker);
			imageViewer->setHightlightedMarker(marker);
		} else {
			fileList->setFilterMode(ImageFileList::FilterNone);
			imageViewer->setHightlightedMarker(0);
		}
	} else {
		imageViewer->setMarkerState(marker, fileList->toggleCurrentImageMarker(marker));
	}
}

void PhotoManagerWindow::exportCurrentImage()
{
	QImage img = imageViewer->currentImage().image();
	if (img.isNull())
		return;

	QString fileName = QFileDialog::getSaveFileName(this, "Export File...", QString());
	if (fileName.isEmpty())
		return;

	// TODO - Allow user to specify scale

	QSize size(img.width() / 2, img.height() / 2);
	img = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	img.save(fileName, nullptr, 98);
}

void PhotoManagerWindow::deleteCurrentImage(bool isShiftActive)
{
	QString imageFile = imageViewer->currentImage().absoluteFilePath();
	if (!imageFile.isEmpty()) {
		if (isShiftActive) {
			if (QMessageBox::question(this, "Permanently Delete File", "Do you want to permanently delete current image?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::StandardButton::Yes) {
				nextFile();
				fileList->reloadFileList();
				QFile::remove(imageFile);
			}
		} else {
			if (QMessageBox::question(this, "Move File to Trash", "Do you want to move current image to trash?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::StandardButton::Yes) {
				nextFile();
				fileList->reloadFileList();
				QFile::moveToTrash(imageFile);
			}
		}
	}
}