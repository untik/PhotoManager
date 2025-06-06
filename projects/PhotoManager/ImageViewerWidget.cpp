#include "ImageViewerWidget.h"
#include <QImageReader>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QMovie>
#include <QElapsedTimer>
#include <QWheelEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDebug>

ImageViewerWidget::ImageViewerWidget(QWidget* parent)
	: QWidget(parent)
{
	imageZoomLevel = 1.0;
	imageRotation = 0;
	isMouseMovementActive = false;
	showImageInformation = true;
	currentImageNumber = 0;
	currentImageCount = 0;
	connect(&animationTimer, &QTimer::timeout, this, &ImageViewerWidget::switchToNextAnimationFrame);
	animationTimer.setSingleShot(true);
}

ImageViewerWidget::~ImageViewerWidget()
{
	invalidateCache();
}

void ImageViewerWidget::setImage(const Image& image)
{
	invalidateCache();
	baseImage = image;
	imageRotation = 0;
	imageOffset = QPoint(0, 0);

	if (baseImage.type() == Image::Type::Vector) {
		// Create renderer when first used
		if (svgRenderer == nullptr) {
			svgRenderer = new QSvgRenderer(baseImage.absoluteFilePath());
			svgScaleX = svgRenderer->viewBoxF().width() / svgRenderer->defaultSize().width();
			svgScaleY = svgRenderer->viewBoxF().height() / svgRenderer->defaultSize().height();
		}
	}

	if (baseImage.frameCount() > 1 && baseImage.type() == Image::Type::Movie) {
		int delay = baseImage.currentFrameDelay();
		if (delay > 0)
			animationTimer.start(delay);
	}

	if (imageInitialZoomLocked) {
		recalculateCachedPixmap();
		update();
		return;
	}

	QSize viewportSize(this->size());
	if (image.size().width() <= viewportSize.width() && image.size().height() <= viewportSize.height())
		zoom(ZoomOriginalSize);
	else
		zoom(ZoomFitToScreen);
}

void ImageViewerWidget::setImageNumber(int number, int total)
{
	currentImageNumber = number;
	currentImageCount = total;
}

void ImageViewerWidget::zoom(ZoomOperation zoomOperation)
{
	static const double Tolerance = 0.0001;

	// Screen fit scale
	QSize viewportSize(this->size());
	QSize pixmapSize(baseImage.size());
	double scale1 = viewportSize.width() / (double)pixmapSize.width();
	double scale2 = viewportSize.height() / (double)pixmapSize.height();
	double scaleFit = qMin(scale1, scale2);

	QVector<double> zoomValues;
	zoomValues << 0.015 << 0.03 << 0.06 << 0.125 << 0.25 << 0.5 << 0.75 << 1 << 1.5 << 2 << 3 << 4 << 5.5 << 8 << 11 << 16;

	bool containsCurrent = false;
	bool containsScaleFit = false;
	for (int i = 0; i < zoomValues.count(); i++) {
		const double value = zoomValues.at(i);
		if (std::abs(value - imageZoomLevel) <= Tolerance)
			containsCurrent = true;
		if (std::abs(value - scaleFit) <= Tolerance)
			containsScaleFit = true;
	}
	if (qFuzzyCompare(imageZoomLevel, scaleFit)) {
		if (!containsCurrent && !containsScaleFit)
			zoomValues.append(imageZoomLevel);
	} else {
		if (!containsCurrent)
			zoomValues.append(imageZoomLevel);
		if (!containsScaleFit)
			zoomValues.append(scaleFit);
	}

	std::sort(zoomValues.begin(), zoomValues.end());
	int currentZoomIndex = findClosestValueIndex(zoomValues, imageZoomLevel);

	switch (zoomOperation) {
		case ImageViewerWidget::ZoomFitToScreen:
			imageZoomLevel = scaleFit;
			break;
		case ImageViewerWidget::ZoomOriginalSize:
			imageZoomLevel = 1.0;
			break;
		case ImageViewerWidget::ZoomIn:
			imageZoomLevel = zoomValues.at(qMin(zoomValues.count() - 1, currentZoomIndex + 1));
			break;
		case ImageViewerWidget::ZoomOut:
			imageZoomLevel = zoomValues.at(qMax(0, currentZoomIndex - 1));
			break;
		case ImageViewerWidget::ZoomQuickToggle:
			if (std::abs(imageZoomLevel - scaleFit) <= Tolerance)
				imageZoomLevel = 1.0;
			else
				imageZoomLevel = scaleFit;
			break;
	}

	recalculateCachedPixmap();
	update();
}

void ImageViewerWidget::rotate(double angle)
{
	baseImage.rotate(angle);
	imageRotation = angle;
	//zoom(ZoomFitToScreen);
	recalculateCachedPixmap();
	update();
}

void ImageViewerWidget::toggleShowImageInformation()
{
	showImageInformation = !showImageInformation;
	update();
}

void ImageViewerWidget::toggleImageInitialZoomLock()
{
	imageInitialZoomLocked = !imageInitialZoomLocked;
}

void ImageViewerWidget::toggleShowHelpText()
{
	showHelpText = !showHelpText;
	update();
}

void ImageViewerWidget::toggleShowDebugInfo()
{
	showDebugInfo = !showDebugInfo;
	update();
}

void ImageViewerWidget::setMarkerState(const QMap<char, bool>& markerState)
{
	imageMarkerState = markerState;
	update();
}

void ImageViewerWidget::setMarkerState(char channel, bool isMarked)
{
	imageMarkerState[channel] = isMarked;
	update();
}

void ImageViewerWidget::setHightlightedMarker(char channel)
{
	highlightedMarker = channel;
	update();
}

void ImageViewerWidget::nextFrame()
{
	if (baseImage.frameCount() == 1)
		return;

	baseImage.jumpToNextImage();
	animationTimer.stop();
	recalculateCachedPixmap();
	update();
}

void ImageViewerWidget::previousFrame()
{
	if (baseImage.frameCount() == 1)
		return;

	baseImage.jumpToPreviousImage();
	animationTimer.stop();
	recalculateCachedPixmap();
	update();
}

void ImageViewerWidget::paintEvent(QPaintEvent* event)
{
	QSize viewportSize(this->size());

	QSize imageSize = preparedImage.image.size();
	QRect centeredRect(QPoint(viewportSize.width() / 2 - imageSize.width() / 2, viewportSize.height() / 2 - imageSize.height() / 2), imageSize);

	centeredRect.translate(preparedImage.renderingOffset);

	bool isMarked = imageMarkerState.value('X', false);

	QPainter painter(this);
	if (isMarked)
		painter.fillRect(QRect(QPoint(0, 0), viewportSize), QColor("#0a0a0a"));
	else
		painter.fillRect(QRect(QPoint(0, 0), viewportSize), QColor("#1e1e1e"));

	if (baseImage.image().isNull()) {
		painter.setFont(QFont("Segoe UI", 20));
		painter.setPen(QColor("#aaa"));
		if (baseImage.fileName().isEmpty())
			painter.drawText(QRect(QPoint(0, 0), viewportSize), "Loading...", QTextOption(Qt::AlignCenter));
		else
			painter.drawText(QRect(QPoint(0, 0), viewportSize), "Unsupported image data format", QTextOption(Qt::AlignCenter));
	}

	painter.drawImage(centeredRect, preparedImage.image);

	if (showHelpText) {
		painter.fillRect(QRect(QPoint(0, 0), viewportSize), QColor::fromRgb(30, 30, 30, 210));
		QTextDocument td;
		td.setMarkdown(applicationHelpText);
		td.setTextWidth(600);
		td.setDefaultFont(QFont("Segoe UI", 12));
		QAbstractTextDocumentLayout::PaintContext ctx;
		ctx.palette.setColor(QPalette::Text, QColor("#ccc"));
		painter.translate(50, 50);
		td.documentLayout()->draw(&painter, ctx);
		return;
	}

	if (showImageInformation && !isMarked)
		renderDescription(&painter, baseImage, viewportSize);

	if (isMarked)
		painter.fillRect(centeredRect, QBrush(QColor(0, 0, 0, 200), Qt::SolidPattern));

	if (showDebugInfo) {
		// Print timing info
		QString debugStr;
		debugStr.append(QString::number(baseImage.elapsedTimeFileLoad()));
		debugStr.append(" ms, ");
		debugStr.append(QString::number(baseImage.elapsedTimeMetadata()));
		debugStr.append(" ms, ");
		debugStr.append(QString::number(baseImage.elapsedTimeBitmapFrames()));
		debugStr.append(" ms, ");
		debugStr.append(QString::number(imageTimeRecalculateCache));
		debugStr.append(" ms, ");
		debugStr.append(QString::number(baseImage.cacheSize()));
		debugStr.append(" MB");

		painter.setFont(QFont("Segoe UI", 12));
		QRect stringBounds = painter.fontMetrics().boundingRect(debugStr);
		QSize rectSize(stringBounds.width() + 10, stringBounds.height() + 10);
		rectSize = rectSize.grownBy(QMargins(10, 5, 10, 5));
		QRect debugInfoRect(QPoint(viewportSize.width() / 2 - rectSize.width() / 2, 0), rectSize);
		painter.fillRect(debugInfoRect, QColor::fromRgb(30, 30, 30, 210));
		painter.drawText(debugInfoRect, debugStr, QTextOption(Qt::AlignCenter));
	}


	//QFont largeFont("Segoe UI", 14);
	//QFontMetrics largeMetrics = QFontMetrics(largeFont);
	//int largeLineHeight = largeMetrics.height();

	//painter.fillRect(QRect(QPoint(0, 0), QSize(50, 1000)), QColor("#0a0a0a"));
	//int buttonOffset = 2;
	//for (int i = 0; i < 10; i++) {
	//	painter.fillRect(QRect(QPoint(1, buttonOffset), QSize(48, 48)), QColor("#1e1e1e"));

	//	if (i == 6) {
	//		painter.fillRect(QRect(QPoint(1, buttonOffset), QSize(48, 48)), QColor("#5e5eAe"));
	//		painter.fillRect(QRect(50, buttonOffset, 200, 48), QColor::fromRgb(30, 30, 30, 180));
	//		painter.setFont(largeFont);
	//		painter.setPen(QColor(Qt::white));
	//		painter.drawText(60, buttonOffset + 30, QString("Button %1 / %2").arg(i).arg(10));
	//	}

	//	buttonOffset += 50;
	//}
}

void ImageViewerWidget::wheelEvent(QWheelEvent* event)
{
	if (event->angleDelta().y() > 0)
		zoom(ZoomIn);
	else if (event->angleDelta().y() < 0)
		zoom(ZoomOut);
}

void ImageViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (isMouseMovementActive) {
		mouseOffset = event->pos() - mouseOrigin;
		//update();

		//QPoint offset = (mouseOrigin - event->pos());
		//imageOffset = offset / imageZoomLevel;
		//recalculateCachedPixmap();
		//update();

		imageOffset = mouseStartImageOffset - (mouseOffset / imageZoomLevel);
		recalculateOffsetLimit();
		recalculateCachedPixmap();
		update();
		event->accept();
	} else {
		event->ignore();
	}
}

void ImageViewerWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		mouseOrigin = event->pos();
		mouseOffset = QPoint(0, 0);
		mouseStartImageOffset = imageOffset;
		isMouseMovementActive = true;
		event->accept();
	} else {
		event->ignore();
	}
}

void ImageViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		isMouseMovementActive = false;
		mouseOffset = QPoint(0, 0);
		recalculateCachedPixmap();
		update();
		event->accept();
	} else {
		event->ignore();
	}
}

void ImageViewerWidget::resizeEvent(QResizeEvent* event)
{
	recalculateCachedPixmap();
}

void ImageViewerWidget::renderDescription(QPainter* painter, const Image& image, const QSize& viewportSize)
{
	MetadataCollection items = image.metadata();

	int xOffset = 5;
	int yOffset = 0;
	int separator = 12;

	QFont smallFont("Segoe UI", 10);
	QFontMetrics smallMetrics = QFontMetrics(smallFont);
	int smallLineHeight = smallMetrics.height();

	QFont largeFont("Segoe UI", 14);
	QFontMetrics largeMetrics = QFontMetrics(largeFont);
	int largeLineHeight = largeMetrics.height();

	QFont markerFont("Segoe UI", 30, 200);
	int markerLineHeight = QFontMetrics(markerFont).tightBoundingRect("12345").height() + 8;

	// Draw background
	int maxWidth = 150;
	int backgroundBuffer = 10;
	for (int i = 0; i < items.count(); i++) {
		const MetadataItem& item = items.at(i);
		maxWidth = qMax(maxWidth, xOffset + smallMetrics.horizontalAdvance(item.stringValue) + backgroundBuffer);
	}
	maxWidth = qMax(maxWidth, xOffset + smallMetrics.horizontalAdvance(image.fileName()) + backgroundBuffer);
	painter->fillRect(QRect(0, 0, maxWidth, viewportSize.height()), QColor::fromRgb(30, 30, 30, 210));

	// File index
	yOffset += largeLineHeight;
	painter->setFont(largeFont);
	painter->setPen(QColor(Qt::white));
	painter->drawText(xOffset, yOffset, QString("%1 / %2").arg(currentImageNumber).arg(currentImageCount));

	yOffset += separator;

	// Label 'File Name'
	yOffset += smallLineHeight;
	painter->setFont(smallFont);
	painter->setPen(QColor(Qt::gray));
	painter->drawText(xOffset, yOffset, "File Name");

	// File name
	yOffset += smallLineHeight;
	painter->setFont(smallFont);
	painter->setPen(QColor(Qt::white));
	painter->drawText(xOffset, yOffset, image.fileName());

	yOffset += separator;

	// Label 'Markers'
	yOffset += smallLineHeight;
	painter->setFont(smallFont);
	painter->setPen(QColor(Qt::gray));
	painter->drawText(xOffset, yOffset, "Markers");

	// Render markers
	yOffset += markerLineHeight;
	painter->setFont(markerFont);

	int markerOffsetX = xOffset - 3;
	int markerSpacing = 26;

	if (imageMarkerState.value('1', false))
		painter->setPen(QPen(QColor("#9999A3")));
	else
		painter->setPen(QPen(QColor("#333333")));
	painter->drawText(markerOffsetX, yOffset, QString("1"));
	if (highlightedMarker == '1') {
		painter->setPen(QPen(QColor("#9999A3"), 5));
		painter->drawLine(markerOffsetX + 5, yOffset + 5, markerOffsetX + markerSpacing - 4, yOffset + 5);
	}
	markerOffsetX += markerSpacing;

	if (imageMarkerState.value('2', false))
		painter->setPen(QPen(QColor("#1f78b4")));
	else
		painter->setPen(QPen(QColor("#333333")));
	painter->drawText(markerOffsetX, yOffset, QString("2"));
	if (highlightedMarker == '2') {
		painter->setPen(QPen(QColor("#1f78b4"), 5));
		painter->drawLine(markerOffsetX, yOffset + 5, markerOffsetX + markerSpacing - 4, yOffset + 5);
	}
	markerOffsetX += markerSpacing;

	if (imageMarkerState.value('3', false))
		painter->setPen(QPen(QColor("#33a02c")));
	else
		painter->setPen(QPen(QColor("#333333")));
	painter->drawText(markerOffsetX, yOffset, QString("3"));
	if (highlightedMarker == '3') {
		painter->setPen(QPen(QColor("#33a02c"), 5));
		painter->drawLine(markerOffsetX, yOffset + 5, markerOffsetX + markerSpacing - 4, yOffset + 5);
	}
	markerOffsetX += markerSpacing;

	if (imageMarkerState.value('4', false))
		painter->setPen(QPen(QColor("#e31a1c")));
	else
		painter->setPen(QPen(QColor("#333333")));
	painter->drawText(markerOffsetX, yOffset, QString("4"));
	if (highlightedMarker == '4') {
		painter->setPen(QPen(QColor("#e31a1c"), 5));
		painter->drawLine(markerOffsetX, yOffset + 5, markerOffsetX + markerSpacing - 4, yOffset + 5);
	}
	markerOffsetX += markerSpacing;

	if (imageMarkerState.value('5', false))
		painter->setPen(QPen(QColor("#ff7f00")));
	else
		painter->setPen(QPen(QColor("#333333")));
	painter->drawText(markerOffsetX, yOffset, QString("5"));
	if (highlightedMarker == '5') {
		painter->setPen(QPen(QColor("#ff7f00"), 5));
		painter->drawLine(markerOffsetX, yOffset + 5, markerOffsetX + markerSpacing - 4, yOffset + 5);
	}
	markerOffsetX += markerSpacing;

	yOffset += separator;

	//// Label 'Path'
	//yOffset += smallLineHeight;
	//painter->setFont(smallFont);
	//painter->setPen(QColor(Qt::gray));
	//painter->drawText(xOffset, yOffset, "Path");

	//// Current file path
	//yOffset += smallLineHeight;
	//painter->setPen(QColor(Qt::white));
	//painter->drawText(xOffset, yOffset, "2018-05-03 Very Long Album Name (Day 1)");

	//yOffset += separator;

	// Label 'Size'
	yOffset += smallLineHeight;
	painter->setFont(smallFont);
	painter->setPen(QColor(Qt::gray));
	painter->drawText(xOffset, yOffset, "Size");

	// Image size
	yOffset += smallLineHeight;
	painter->setFont(smallFont);
	painter->setPen(QColor(Qt::white));
	int megaPixels = int((double(image.size().width() * image.size().height()) / 1e6) + 0.5);
	if (megaPixels > 0)
		painter->drawText(xOffset, yOffset, QString("%1 x %2 (%3M)").arg(image.size().width()).arg(image.size().height()).arg(megaPixels));
	else
		painter->drawText(xOffset, yOffset, QString("%1 x %2").arg(image.size().width()).arg(image.size().height()));

	// Current image scale
	yOffset += smallLineHeight;
	painter->drawText(xOffset, yOffset, QString("%1 %").arg(imageZoomLevel * 100.0));

	// Animation info
	if (baseImage.frameCount() > 1) {
		yOffset += separator;

		// Label 'Frames'
		yOffset += smallLineHeight;
		painter->setFont(smallFont);
		painter->setPen(QColor(Qt::gray));
		painter->drawText(xOffset, yOffset, "Frame Count");

		// Animation frame count
		yOffset += smallLineHeight;
		painter->setFont(smallFont);
		painter->setPen(QColor(Qt::white));
		painter->drawText(xOffset, yOffset, QString("%1 / %2").arg(baseImage.currentFrameIndex() + 1).arg(baseImage.frameCount()));
	}

	// No metadata warning
	if (items.isEmpty()) {
		yOffset += smallLineHeight + separator;
		painter->drawText(xOffset, yOffset, "No Metadata");
		return;
	}

	// Print embedded image metadata
	for (int i = 0; i < items.count(); i++) {
		const MetadataItem& item = items.at(i);
		// Label
		yOffset += smallLineHeight + separator;
		painter->setPen(QColor(Qt::gray));
		painter->drawText(xOffset, yOffset, item.label);
		// Value
		yOffset += smallLineHeight;
		painter->setPen(QColor(Qt::white));
		painter->drawText(xOffset, yOffset, item.stringValue);
	}


	//// Render markers
	//painter->setFont(QFont("Segoe UI", 90, 200));
	////int markerLineHeight = painter->fontMetrics().height() * 0.6;
}

void ImageViewerWidget::recalculateOffsetLimit()
{
	double scale = imageZoomLevel;

	//Visible target area size (1920 x 1080)
	QSize viewportSize(this->size());

	// Original image size (5472 x 3648)
	QSize imageSize(baseImage.size());

	// Size of full image when scaled
	QSize scaledSize(imageSize * scale);

	// Size of area in original image needed to fill viewport
	QSize sourceAreaSize = viewportSize / scale;
	if (sourceAreaSize.width() > imageSize.width())
		sourceAreaSize.setWidth(imageSize.width());
	if (sourceAreaSize.height() > imageSize.height())
		sourceAreaSize.setHeight(imageSize.height());

	imageOffset.setX(qBound(-imageSize.width() / 2 + sourceAreaSize.width() / 2, imageOffset.x(), imageSize.width() / 2 - sourceAreaSize.width() / 2));
	imageOffset.setY(qBound(-imageSize.height() / 2 + sourceAreaSize.height() / 2, imageOffset.y(), imageSize.height() / 2 - sourceAreaSize.height() / 2));
}

void ImageViewerWidget::recalculateCachedPixmap()
{
	QElapsedTimer timer;
	timer.start();

	//QImage baseImageData= baseImage.image();

	double scale = imageZoomLevel;

	Qt::TransformationMode mode = Qt::SmoothTransformation;
	if (scale >= 1)
		mode = Qt::FastTransformation;

	//Visible target area size (1920 x 1080)
	QSize viewportSize(this->size());

	// Original image size (5472 x 3648)
	QSize imageSize(baseImage.size());

	// Size of full image when scaled
	QSize scaledSize(imageSize * scale);

	// Size of area in original image needed to fill viewport
	QSize sourceAreaSize = viewportSize / scale;

	// Limit maximum source area to full image size
	if (sourceAreaSize.width() > imageSize.width())
		sourceAreaSize.setWidth(imageSize.width());
	if (sourceAreaSize.height() > imageSize.height())
		sourceAreaSize.setHeight(imageSize.height());

	// Point in original image which should be centered in viewport
	QPoint centerPoint(imageSize.width() / 2, imageSize.height() / 2);
	centerPoint = centerPoint + imageOffset;

	// Ensure center point lies inside the original image
	centerPoint.setX(qBound(0, centerPoint.x(), imageSize.width()));
	centerPoint.setY(qBound(0, centerPoint.y(), imageSize.height()));

	// Calculate origin point for clipped rectangle inside original image
	QPoint sourceTopLeft(centerPoint.x() - sourceAreaSize.width() / 2, centerPoint.y() - sourceAreaSize.height() / 2);

	sourceTopLeft.setX(qBound(0, sourceTopLeft.x(), imageSize.width() - sourceAreaSize.width()));
	sourceTopLeft.setY(qBound(0, sourceTopLeft.y(), imageSize.height() - sourceAreaSize.height()));

	// Rectangle area specifying needed pixels from original image (may only partially intersect with original image)
	QRect sourceAreaRect(sourceTopLeft, sourceAreaSize);

	// Limit rect to original image rect
	int xp1, yp1, xp2, yp2;
	int oxp1, oyp1, oxp2, oyp2;
	sourceAreaRect.getCoords(&oxp1, &oyp1, &oxp2, &oyp2);
	xp1 = qMax(oxp1, 0);
	yp1 = qMax(oyp1, 0);
	xp2 = qMin(oxp2, imageSize.width());
	yp2 = qMin(oyp2, imageSize.height());

	QRect limitedSourceAreaRect;
	limitedSourceAreaRect.setCoords(xp1, yp1, xp2, yp2);

	//// Offset for rendering scaled image at correct position
	//QPoint renderingOffset(std::abs(oxp1 - xp1), std::abs(oyp1 - yp1));
	//renderingOffset = renderingOffset / scale;

	// Size of the final scaled image
	QSize targetSize = viewportSize;
	if (scaledSize.width() < viewportSize.width())
		targetSize.setWidth(scaledSize.width());
	if (scaledSize.height() < viewportSize.height())
		targetSize.setHeight(scaledSize.height());

	//preparedImage.renderingOffset = renderingOffset;

	// Test if prepared image data needs to be recalculated
	bool refreshNeededAnimation = (baseImage.type() == Image::Type::Movie && animationPlayerPreparedFrame != baseImage.currentFrameIndex());
	bool refreshNeededSize = (preparedImage.image.size() != targetSize || preparedImage.sourceRect != limitedSourceAreaRect);
	if (!refreshNeededAnimation && !refreshNeededSize) {
		//qDebug() << "Rendering skipped";
		return;
	}

	if (baseImage.type() == Image::Type::Vector) {
		// Alocate image for painting
		if (preparedImage.image.size() != targetSize)
			preparedImage.image = QImage(targetSize, QImage::Format_ARGB32_Premultiplied);
		preparedImage.image.fill(QColor("#fff"));
		QPainter p(&preparedImage.image);

		svgRenderer->setViewBox(QRectF(limitedSourceAreaRect.x() * svgScaleX, limitedSourceAreaRect.y() * svgScaleY, limitedSourceAreaRect.width() * svgScaleX, limitedSourceAreaRect.height() * svgScaleY));
		svgRenderer->setAspectRatioMode(Qt::KeepAspectRatio);
		svgRenderer->render(&p, QRectF(0, 0, targetSize.width(), targetSize.height()));

		//qDebug() << "SVG rendered in:" << timer.elapsed() << "ms";
	}
	else {
		QImage clipped = baseImage.image().copy(limitedSourceAreaRect);

		int prescaling = 1;
		if (optimize && mode == Qt::SmoothTransformation) {
			if (clipped.size().width() >= 8 * targetSize.width() && clipped.size().height() >= 8 * targetSize.height()) {
				clipped = clipped.scaled(clipped.size() / 4);
				prescaling = 4;
			} else if (clipped.size().width() >= 4 * targetSize.width() && clipped.size().height() >= 4 * targetSize.height()) {
				clipped = clipped.scaled(clipped.size() / 2);
				prescaling = 2;
			}
		}

		preparedImage.image = clipped.scaled(targetSize, Qt::KeepAspectRatio, mode);
		//preparedImage.image = originalImage.copy(limitedSourceAreaRect).scaled(targetSize, Qt::KeepAspectRatio, mode);
		preparedImage.sourceRect = limitedSourceAreaRect;

		//qDebug().nospace() << "Scaled in: " << timer.elapsed() << " ms, Prescaling: " << prescaling << "x";
	}

	imageTimeRecalculateCache = timer.elapsed();
}

void ImageViewerWidget::invalidateCache()
{
	preparedImage.image = QImage();
	preparedImage.sourceRect = QRect();
	if (svgRenderer != nullptr) {
		delete svgRenderer;
		svgRenderer = nullptr;
	}
	animationTimer.stop();
}

int ImageViewerWidget::findClosestValueIndex(const QVector<double>& values, double x)
{
	int minDistanceIndex = 0;
	double minDistance = std::abs(values.first() - x);
	for (int i = 1; i < values.count(); i++) {
		double distance = std::abs(values.at(i) - x);
		if (distance < minDistance) {
			minDistance = distance;
			minDistanceIndex = i;
		}
	}
	return minDistanceIndex;
}

void ImageViewerWidget::switchToNextAnimationFrame()
{
	baseImage.jumpToNextImage();
	int delay = baseImage.currentFrameDelay();
	if (delay > 0)
		animationTimer.start(delay);

	recalculateCachedPixmap();
	update();
}
