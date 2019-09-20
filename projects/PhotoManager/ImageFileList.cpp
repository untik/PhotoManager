#include "ImageFileList.h"
#include <QFileInfo>
#include <QDir>

ImageFileList::ImageFileList(QObject* parent)
	: QObject(parent), fileListCurrentIndex(-1), isWrapAroundAllowed(true), fileListFilteredCount(0)
{
	fileListFilterMode = FilterNone;
	fileListNameFilters << "*.jpg" << "*.png";

	markerFiles.insert(Marker1, new MarkerFile("selection-1.txt", Marker1)); // *.photolist
	markerFiles.insert(Marker2, new MarkerFile("selection-2.txt", Marker2));
	markerFiles.insert(Marker3, new MarkerFile("selection-3.txt", Marker3));
	markerFiles.insert(Marker4, new MarkerFile("selection-4.txt", Marker4));
	markerFiles.insert(Marker5, new MarkerFile("selection-5.txt", Marker5));
	markerFiles.insert(MarkerDelete, new MarkerFile("selection-X.txt", MarkerDelete));
}

void ImageFileList::setCurrentFile(const QString& fileName)
{
	QFileInfo fileInfo;
	if (fileName.contains('/')) {
		fileInfo = QFileInfo(fileName);

		// Check if new working directory is set
		QString fileDirectory = fileInfo.absolutePath();
		if (fileListWorkingDirectory != fileDirectory) {
			fileListWorkingDirectory = fileDirectory;
			reloadFileList();
			reloadMarkerFiles();
		}
	} else {
		fileInfo = QFileInfo(fileListWorkingDirectory + '/' + fileName);
	}

	// Find current file in file list
	QString currentFileName = fileInfo.fileName();
	for (int i = 0; i < fileListItems.count(); i++) {
		const Item& item = fileListItems.at(i);
		if (item.fileName == currentFileName) {
			fileListCurrentIndex = i;
			break;
		}
	}
}

void ImageFileList::reloadFileList()
{
	QString currentFileName;
	if (fileListCurrentIndex != -1)
		currentFileName = fileListItems.at(fileListCurrentIndex).fileName;

	fileListItems.clear();

	QDir dir(fileListWorkingDirectory);
	QFileInfoList list = dir.entryInfoList(fileListNameFilters, QDir::Files, QDir::Name | QDir::IgnoreCase);
	for (int i = 0; i < list.count(); i++) {
		const QFileInfo& fi = list.at(i);
		Item item;
		item.fullFilePath = fi.absoluteFilePath();
		item.fileName = fi.fileName();
		fileListItems.append(item);
	}

	fileListCurrentIndex = 0;
	for (int i = 0; i < fileListItems.count(); i++) {
		const Item& item = fileListItems.at(i);
		if (item.fileName == currentFileName) {
			fileListCurrentIndex = i;
			break;
		}
	}
	copyMarkersFromFiles();
	applyFilters();
}

void ImageFileList::reloadMarkerFiles()
{
	QMapIterator<int, MarkerFile*> i(markerFiles);
	while (i.hasNext()) {
		i.next();
		MarkerFile* markerFile = i.value();
		markerFile->loadDirectory(fileListWorkingDirectory);
	}
	copyMarkersFromFiles();
}

void ImageFileList::copyMarkersFromFiles()
{
	QMapIterator<int, MarkerFile*> i(markerFiles);
	while (i.hasNext()) {
		i.next();
		MarkerFile* markerFile = i.value();

		for (int i = 0; i < fileListItems.count(); i++) {
			Item& item = fileListItems[i];
			item.markers[markerFile->type()] = markerFile->isMarked(item.fullFilePath);
		}
	}
}

ImageFileList::Item ImageFileList::fileAtOffset(int offset)
{
	if (fileListItems.isEmpty() || fileListCurrentIndex == -1)
		return Item();

	if (offset == 0)
		return fileListItems.at(fileListCurrentIndex);

	int absOffset = abs(offset);
	int direction = (offset > 0) ? 1 : -1;

	int lastIndex = fileListCurrentIndex;
	for (int i = 0; i < absOffset; i++) {
		// Find next unfiltered file
		int nextIndex = nextUnfileredFile(lastIndex, direction, isWrapAroundAllowed);
		if (nextIndex == -1)
			return fileListItems.at(fileListCurrentIndex);
		lastIndex = nextIndex;
	}
	return fileListItems.at(lastIndex);
}

int ImageFileList::fileCount() const
{
	//int count = 0;
	//for (int i = 0; i < fileListItems.count(); i++) {
	//	const Item& item = fileListItems.at(i);
	//	if (item.isPassingFilter)
	//		count++;
	//}
	return fileListFilteredCount;
}

int ImageFileList::unfilteredFileCount() const
{
	return fileListItems.count();
}

void ImageFileList::setFilterMode(FilterMode filterMode, MarkerType markerType)
{
	setFilterMode(filterMode, QVector<MarkerType>() << markerType);
}

void ImageFileList::setFilterMode(FilterMode filterMode, const QVector<MarkerType>& markerTypes)
{
	fileListFilterMode = filterMode;
	fileListFilterMarkers = markerTypes;
	applyFilters();
}

ImageFileList::FilterMode ImageFileList::currentFilterMode() const
{
	return fileListFilterMode;
}

void ImageFileList::setAllowWrapAround(bool isAllowed)
{
	isWrapAroundAllowed = isAllowed;
}

void ImageFileList::setFileNameFilters(const QStringList& nameFilters)
{
	fileListNameFilters = nameFilters;
	reloadFileList();
}

void ImageFileList::setCurrentImageMarkers(const QVector<MarkerType>& markerTypes)
{
	if (fileListCurrentIndex == -1)
		return;
	Item& currentItem = fileListItems[fileListCurrentIndex];

	QMapIterator<int, MarkerFile*> i(markerFiles);
	while (i.hasNext()) {
		i.next();
		MarkerFile* markerFile = i.value();
		bool isMarked = markerTypes.contains(markerFile->type());

		currentItem.markers[markerFile->type()] = isMarked;
		markerFile->setMarkedState(currentItem.fullFilePath, isMarked);
	}
}

bool ImageFileList::toggleCurrentImageMarker(MarkerType markerType)
{
	if (fileListCurrentIndex == -1)
		return false;

	Item& item = fileListItems[fileListCurrentIndex];

	bool isMarked = !item.markers.value(markerType, false);
	item.markers[markerType] = isMarked;
	markerFiles[markerType]->setMarkedState(item.fullFilePath, isMarked);
	return isMarked;
}

void ImageFileList::applyFilters()
{
	fileListFilteredCount = 0;
	if (fileListFilterMode == FilterNone) {
		for (int i = 0; i < fileListItems.count(); i++) {
			Item& item = fileListItems[i];
			item.isPassingFilter = true;
			item.filteredIndex = fileListFilteredCount;
			fileListFilteredCount++;
		}
	}
	else if (fileListFilterMode == FilterSingleMarker) {
		for (int i = 0; i < fileListItems.count(); i++) {
			Item& item = fileListItems[i];
			item.isPassingFilter = false;
			for (int j = 0; j < fileListFilterMarkers.count(); j++) {
				if (item.markers.value(fileListFilterMarkers.at(j), false)) {
					item.isPassingFilter = true;
					item.filteredIndex = fileListFilteredCount;
					fileListFilteredCount++;
					break;
				}
			}
		}
	}
	else if (fileListFilterMode == FilterHideMarker) {
		for (int i = 0; i < fileListItems.count(); i++) {
			Item& item = fileListItems[i];
			item.isPassingFilter = true;
			for (int j = 0; j < fileListFilterMarkers.count(); j++) {
				if (item.markers.value(fileListFilterMarkers.at(j), false)) {
					item.isPassingFilter = false;
					break;
				}
			}
			if (item.isPassingFilter) {
				item.filteredIndex = fileListFilteredCount;
				fileListFilteredCount++;
			}
		}
	}
}

int ImageFileList::nextUnfileredFile(int index, int direction, bool wrap)
{
	int itemCount = fileListItems.count();
	for (int i = 1; i < itemCount; i++) {
		int currentIndex;
		if (wrap) {
			currentIndex = ((index + (i * direction)) + itemCount) % itemCount;
		} else {
			currentIndex = index + (i * direction);
			if (currentIndex >= itemCount || currentIndex < 0)
				return -1;
		}

		if (fileListItems.at(currentIndex).isPassingFilter)
			return currentIndex;
	}
	return -1;
}
