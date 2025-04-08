#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include "MarkerFile.h"
#include "MarkerType.h"

class ImageFileList : public QObject
{
	Q_OBJECT

public:
	enum FilterMode
	{
		FilterNone,
		FilterSingleMarker,
		FilterHideMarker,
	};

	struct Item
	{
		QString fullFilePath;
		QString fileName;
		QMap<char, bool> markers;
		bool isPassingFilter = true;
		int filteredIndex = 0;

		Item() {}
		bool isValid() const { return !fullFilePath.isEmpty(); }
	};

	ImageFileList(QObject* parent = nullptr);

	// Load image and marker files from directory containing specified file. Set the file as current image.
	void setCurrentFile(const QString& fileName);

	// Reload files from current working directory. Try to preserve current image.
	void reloadFileList();

	// Reload marker files from current working directory.
	void reloadMarkerFiles();

	// Returns image at specified offset from current image.
	Item fileAtOffset(int offset);

	// Returns number of available images in loaded file list. Takes filters into account.
	int fileCount() const;

	// Returns number of available images in loaded file list. Ignores filters.
	int unfilteredFileCount() const;

	// Returns index of current file
	int currentIndex() const { return fileListCurrentIndex; }

	// Set filters for files returned by fileAtOffset.
	void setFilterMode(FilterMode filterMode, MarkerType markerType = MarkerUnmarked);
	void setFilterMode(FilterMode filterMode, const QVector<MarkerType>& markerTypes);
	FilterMode currentFilterMode() const;

	// When true fileAtOffset starts from the beginning of the list after reaching the end.
	void setAllowWrapAround(bool isAllowed);

	// Set allowed file types
	void setFileNameFilters(const QStringList& nameFilters);

	// Modify markers for current image.
	void setCurrentImageMarkers(const QVector<MarkerType>& markerTypes);

	// Toggles state of specified marker. Returns true if marker is toggled on.
	bool toggleCurrentImageMarker(MarkerType markerType);

private:
	void copyMarkersFromFiles();
	void applyFilters();
	int nextUnfileredFile(int index, int direction, bool wrap);

private:
	QString fileListWorkingDirectory;
	QVector<Item> fileListItems;
	int fileListCurrentIndex;
	FilterMode fileListFilterMode;
	QVector<MarkerType> fileListFilterMarkers;
	QStringList fileListNameFilters;
	bool isWrapAroundAllowed;
	QMap<int, MarkerFile*> markerFiles;
	int fileListFilteredCount;
};
