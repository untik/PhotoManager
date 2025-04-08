#pragma once

#include <QString>
#include <QVector>
#include "MetadataItem.h"
#include "MetadataCollection.h"

class MetadataReader
{
public:
	MetadataReader();
	~MetadataReader();

	static MetadataCollection load(const QString& fileName);

private:
	static QString decodeExposureProgram(int code);
	static QString decodeMeteringMode(int code);
	static QString decodeFlash(int code);
	static QString decodeExposureMode(int code);
	static QString decodeWhiteBalance(int code);
	static QString formatExposureTime(double v1, double v2);
	static QString formatDateTime(QString exifString);
	static QString decodePanasonicShootingMode(int code);
	static QString decodePanasonicAdvancedSceneMode(int shootingMode, int advancedSceneType);
	static QString decodePanasonicBurstMode(int code);
	static QString decodePanasonicSelfTimer(int code);
	static QString decodePanasonicHdr(int code);
	static QString decodePanasonicBracketSettings(int code);
	static QString decodeImageCompression(int code);
};
