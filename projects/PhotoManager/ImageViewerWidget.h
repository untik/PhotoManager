#pragma once

#include <QWidget>
#include <QMap>
#include "Image.h"

class QSvgRenderer;
class QMovie;

class ImageViewerWidget : public QWidget
{
	Q_OBJECT

public:
	enum ZoomOperation
	{
		ZoomFitToScreen,
		ZoomOriginalSize,
		ZoomIn,
		ZoomOut,
		ZoomQuickToggle
	};

	ImageViewerWidget(QWidget* parent = nullptr);
	~ImageViewerWidget();

	void setImage(const Image& image);
	void setImageNumber(int number, int total);

	const Image& currentImage() const { return baseImage; }

	void zoom(ZoomOperation zoomOperation);
	void rotate(double angle);

	void toggleShowImageInformation();

	void setMarkerState(const QMap<char, bool>& markerState);
	void setMarkerState(char channel, bool isMarked);
	void setHightlightedMarker(char channel);

	void recalculate() { invalidateCache(); recalculateCachedPixmap(); update(); }
	bool optimize = true;

protected:
	void paintEvent(QPaintEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	void renderDescription(QPainter* painter, const Image& image, const QSize& viewportSize);
	void recalculateOffsetLimit();
	void recalculateCachedPixmap();
	void invalidateCache();
	int findClosestValueIndex(const QVector<double>& values, double x);

	void movieFrameChanged(int frameNumber);

private:
	struct PreparedImage
	{
		QImage image;
		QRect sourceRect;
		QPoint renderingOffset;
	};

	Image baseImage;
	PreparedImage preparedImage;
	QSvgRenderer* svgRenderer = nullptr;
	double svgScaleX;
	double svgScaleY;
	QMovie* animationPlayer = nullptr;
	int animationPlayerPreparedFrame = -1;

	double imageZoomLevel;
	double imageRotation;
	QPoint imageOffset;

	bool isMouseMovementActive;
	QPoint mouseStartImageOffset;
	QPoint mouseOffset;
	QPoint mouseOrigin;

	bool showImageInformation;
	QMap<char, bool> imageMarkerState;
	char highlightedMarker;

	int currentImageNumber;
	int currentImageCount;
};
