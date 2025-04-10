#include "MetadataReader.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "exiv2\exiv2.hpp"
#pragma comment(lib, "exiv2.lib")


MetadataReader::MetadataReader()
{}

MetadataReader::~MetadataReader()
{}

MetadataCollection MetadataReader::load(const QByteArray& fileData, const QString& fileType)
{
	// Catch unsupported file extensions
	if (fileType == "ico" || fileType == "svg" || fileType == "tga")
		return MetadataCollection();

	MetadataCollection items;

	try {
		Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(reinterpret_cast<const uint8_t*>(fileData.constData()), fileData.size());
		if (image.get() == 0)
			return MetadataCollection();

		image->readMetadata();

		Exiv2::ExifData& exifData = image->exifData();
		Exiv2::ExifData::const_iterator exifEnd = exifData.end();
		for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifEnd; ++i) {
			const char* tn = i->typeName();
			QByteArray key = QByteArray::fromStdString(i->key());
			QByteArray val = QByteArray::fromStdString(i->toString());
			qDebug() << key << i->tag() << (tn ? tn : "Unknown") << i->count() << val;

			if (key == "Exif.Image.Make") {
				MetadataItem item("Make", key);
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
			else if (key == "Exif.Image.Model") {
				MetadataItem item("Model", key);
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
			else if (key == "Exif.Image.Software") {
				MetadataItem item("Software", key);
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
			else if (key == "Exif.Image.Artist") {
				MetadataItem item("Artist", key);
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
			else if (key == "Exif.Image.Compression") {
				MetadataItem item("Compression", key);
				item.stringValue = decodeImageCompression(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Photo.DateTimeOriginal") {
				MetadataItem item("DateTime", key);
				item.stringValue = formatDateTime(QString::fromStdString(i->toString()));
				items.append(item);
			}
			else if (key == "Exif.Photo.ISOSpeedRatings") {
				MetadataItem item("ISO", key);
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
			else if (key == "Exif.Photo.ExposureTime") {
				MetadataItem item("Exposure", key);
				Exiv2::Rational val = i->toRational();
				item.stringValue = QString("%1 s").arg(formatExposureTime(val.first, val.second));
				item.unit = "s";
				items.append(item);
			}
			else if (key == "Exif.Photo.FNumber") {
				MetadataItem item("Aperture", key);
				Exiv2::Rational val = i->toRational();
				item.stringValue = QString("ƒ/%1").arg((double)val.first / val.second);
				items.append(item);
			}
			else if (key == "Exif.Photo.ExposureProgram") {
				MetadataItem item("Program", key);
				item.stringValue = decodeExposureProgram(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Photo.MeteringMode") {
				MetadataItem item("Metering Mode", key);
				item.stringValue = decodeMeteringMode(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Photo.Flash") {
				MetadataItem item("Flash", key);
				item.stringValue = decodeFlash(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Photo.ExposureMode") {
				MetadataItem item("Exposure Mode", key);
				item.stringValue = decodeExposureMode(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Photo.WhiteBalance") {
				MetadataItem item("White Balance", key);
				item.stringValue = decodeWhiteBalance(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Photo.FocalLengthIn35mmFilm") {
				MetadataItem item("Focal Length (35mm)", key);
				item.stringValue = QString("%1 mm").arg(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Panasonic.ColorTempKelvin") {
				MetadataItem item("Color Temperature", key);
				item.stringValue = QString("%1 K").arg(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Panasonic.ShootingMode") {
				MetadataItem item("Shooting Mode", key);
				item.rawIntegerValue = i->toUint32();
				item.stringValue = decodePanasonicShootingMode(i->toUint32());
				items.append(item);
			}
			else if (key == "Exif.Panasonic.AdvancedSceneType") {
				MetadataItem item("Advanced Scene Type", key);
				item.rawIntegerValue = i->toUint32();
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
			else if (key == "Exif.Panasonic.BurstMode") {
				MetadataItem item("Burst Mode", key);
				item.rawIntegerValue = i->toUint32();
				if (item.rawIntegerValue > 0) {
					item.stringValue = decodePanasonicBurstMode(i->toUint32());
					items.append(item);
				}
			}
			else if (key == "Exif.Panasonic.SelfTimer") {
				MetadataItem item("Self Timer", key);
				item.rawIntegerValue = i->toUint32();
				if (item.rawIntegerValue > 1) {
					item.stringValue = decodePanasonicSelfTimer(i->toUint32());
					items.append(item);
				}
			}
			else if (key == "Exif.Panasonic.HDR") {
				MetadataItem item("HDR", key);
				item.rawIntegerValue = i->toUint32();
				if (item.rawIntegerValue > 0) {
					item.stringValue = decodePanasonicHdr(i->toUint32());
					items.append(item);
				}
			}
			else if (key == "Exif.Panasonic.BracketSettings") {
				MetadataItem item("Bracket Settings", key);
				item.rawIntegerValue = i->toUint32();
				if (item.rawIntegerValue > 0) {
					item.stringValue = decodePanasonicBracketSettings(i->toUint32());
					items.append(item);
				}
			}
		}
		const int metadataExifCount = items.count();

		Exiv2::XmpData& xmpData = image->xmpData();
		Exiv2::XmpData::const_iterator xmpEnd = xmpData.end();
		for (Exiv2::XmpData::const_iterator i = xmpData.begin(); i != xmpEnd; ++i) {
			const char *tn = i->typeName();
			QByteArray key = QByteArray::fromStdString(i->key());
			QByteArray val = QByteArray::fromStdString(i->toString());
			qDebug() << key << i->tag() << (tn ? tn : "Unknown") << i->count() << val;

			if (key == "Xmp.dc.creator") {
				MetadataItem item("Xmp.dc.creator", key);
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
			else if (key == "Xmp.xmp.CreatorTool") {
				MetadataItem item("Xmp.CreatorTool", key);
				item.stringValue = QString::fromStdString(i->toString());
				items.append(item);
			}
		}
		const int metadataXmpCount = items.count() - metadataExifCount;

		Exiv2::IptcData& iptcData = image->iptcData();
		Exiv2::IptcData::const_iterator iptcEnd = iptcData.end();
		for (Exiv2::IptcData::const_iterator i = iptcData.begin(); i != iptcEnd; ++i) {
			const char* tn = i->typeName();
			QByteArray key = QByteArray::fromStdString(i->key());
			QByteArray val = QByteArray::fromStdString(i->toString());
			qDebug() << key << i->tag() << (tn ? tn : "Unknown") << i->count() << val;
		}
		const int metadataIptcCount = items.count() - (metadataExifCount + metadataXmpCount);

		// Show additional metadata indicators
		if (exifData.count() > 0 || xmpData.count() > 0 || iptcData.count() > 0) {
			MetadataItem item("Other Metadata");
			if (exifData.count() > 0) {
				if (!item.stringValue.isEmpty())
					item.stringValue.append(", ");
				item.stringValue.append(QString("EXIF: %1").arg(exifData.count() - metadataExifCount));
			}
			if (xmpData.count() > 0) {
				if (!item.stringValue.isEmpty())
					item.stringValue.append(", ");
				item.stringValue.append(QString("XMP: %1").arg(xmpData.count() - metadataXmpCount));
			}
			if (iptcData.count() > 0) {
				if (!item.stringValue.isEmpty())
					item.stringValue.append(", ");
				item.stringValue.append(QString("IPTC: %1").arg(iptcData.count() - metadataIptcCount));
			}
			items.append(item);
		}
	}
	catch (Exiv2::Error& e) {
		qDebug().nospace() << "Exiv2::Error (" << static_cast<int>(e.code()) << "): " << e.what();
		return MetadataCollection();
	}

	// Postprocessing for combined values
	MetadataItem* panasonicShootingMode = items.findKey("Exif.Panasonic.ShootingMode");
	MetadataItem* panasonicAdvancedSceneType = items.findKey("Exif.Panasonic.AdvancedSceneType");
	if (panasonicShootingMode != nullptr && panasonicAdvancedSceneType != nullptr) {
		panasonicAdvancedSceneType->stringValue = decodePanasonicAdvancedSceneMode(panasonicShootingMode->rawIntegerValue, panasonicAdvancedSceneType->rawIntegerValue);
		if (panasonicAdvancedSceneType->stringValue.isEmpty())
			items.removeKey("Exif.Panasonic.AdvancedSceneType");
	}

	return items;
}

QString MetadataReader::decodeExposureProgram(int code)
{
	switch (code) {
		case 0: return QString("Unknown");
		case 1: return QString("Manual");
		case 2: return QString("Normal");
		case 3: return QString("Aperture Priority");
		case 4: return QString("Shutter Priority");
		case 5: return QString("Creative"); // depth of field bias
		case 6: return QString("Action"); // fast shutter speed bias
		case 7: return QString("Portrait");
		case 8: return QString("Landscape");
	}
	return QString("Other");
}

QString MetadataReader::decodeMeteringMode(int code)
{
	switch (code) {
		case 0: return QString("Unknown");
		case 1: return QString("Average");
		case 2: return QString("Center-weighted");
		case 3: return QString("Spot");
		case 4: return QString("Multi-spot");
		case 5: return QString("Pattern");
		case 6: return QString("Partial");
	}
	return QString("Other");
}

QString MetadataReader::decodeFlash(int code)
{
	int flashFired = code & 0x1;
	int flashReturn = code >> 1 & 0x3;
	int flashMode = code >> 3 & 0x3;
	int flashFunction = code >> 5 & 0x1;
	int flashRedEye = code >> 6 & 0x1;

	QString description;
	if (flashMode == 1)
		description.append("On");
	else if (flashMode == 2)
		description.append("Off");
	else if (flashMode == 3)
		description.append("Auto");

	if (!description.isEmpty()) {
		if (flashFired == 0)
			description.append(", Not Fired");
		else
			description.append(", Fired");

		if (flashRedEye == 1)
			description.append(", RedEye");
	} else {
		return QString::number(code);
	}
	return description;
}

QString MetadataReader::decodeExposureMode(int code)
{
	switch (code) {
		case 0: return QString("Auto");
		case 1: return QString("Manual");
		case 2: return QString("Auto Bracket");
	}
	return QString("Other");
}

QString MetadataReader::decodeWhiteBalance(int code)
{
	switch (code) {
		case 0: return QString("Auto");
		case 1: return QString("Manual");
	}
	return QString("Other");
}

QString MetadataReader::formatExposureTime(double v1, double v2)
{
	if (v1 / v2 >= 1) {
		return QString::number(v1 / v2);
	} else {
		return QString("%1/%2").arg(v1 / v1).arg(v2 / v1);
	}
}

QString MetadataReader::formatDateTime(QString exifString)
{
	// 2016:10:10 10:10:10
	if (exifString.length() < 19)
		return exifString;

	exifString[4] = '-';
	exifString[7] = '-';
	return exifString;
}

QString MetadataReader::decodePanasonicShootingMode(int code)
{
	switch (code) {
		case 1: return QString("Normal");
		case 2: return QString("Portrait");
		case 3: return QString("Scenery");
		case 4: return QString("Sports");
		case 5: return QString("Night Portrait");
		case 6: return QString("Program");
		case 7: return QString("Aperture Priority");
		case 8: return QString("Shutter Priority");
		case 9: return QString("Macro");
		case 10: return QString("Spot");
		case 11: return QString("Manual");
		case 12: return QString("Movie Preview");
		case 13: return QString("Panning");
		case 14: return QString("Simple");
		case 15: return QString("Color Effects");
		case 16: return QString("Self Portrait");
		case 17: return QString("Economy");
		case 18: return QString("Fireworks");
		case 19: return QString("Party");
		case 20: return QString("Snow");
		case 21: return QString("Night Scenery");
		case 22: return QString("Food");
		case 23: return QString("Baby");
		case 24: return QString("Soft Skin");
		case 25: return QString("Candlelight");
		case 26: return QString("Starry Night");
		case 27: return QString("High Sensitivity");
		case 28: return QString("Panorama Assist");
		case 29: return QString("Underwater");
		case 30: return QString("Beach");
		case 31: return QString("Aerial Photo");
		case 32: return QString("Sunset");
		case 33: return QString("Pet");
		case 34: return QString("Intelligent ISO");
		case 35: return QString("Clipboard");
		case 36: return QString("High Speed Continuous Shooting");
		case 37: return QString("Intelligent Auto");
		case 39: return QString("Multi - aspect");
		case 41: return QString("Transform");
		case 42: return QString("Flash Burst");
		case 43: return QString("Pin Hole");
		case 44: return QString("Film Grain");
		case 45: return QString("My Color");
		case 46: return QString("Photo Frame");
		case 48: return QString("Movie");
		case 51: return QString("HDR");
		case 52: return QString("Peripheral Defocus");
		case 55: return QString("Handheld Night Shot");
		case 57: return QString("3D");
		case 59: return QString("Creative Control");
		case 62: return QString("Panorama");
		case 63: return QString("Glass Through");
		case 64: return QString("HDR");
		case 66: return QString("Digital Filter");
		case 67: return QString("Clear Portrait");
		case 68: return QString("Silky Skin");
		case 69: return QString("Backlit Softness");
		case 70: return QString("Clear in Backlight");
		case 71: return QString("Relaxing Tone");
		case 72: return QString("Sweet Child's Face");
		case 73: return QString("Distinct Scenery");
		case 74: return QString("Bright Blue Sky");
		case 75: return QString("Romantic Sunset Glow");
		case 76: return QString("Vivid Sunset Glow");
		case 77: return QString("Glistening Water");
		case 78: return QString("Clear Nightscape");
		case 79: return QString("Cool Night Sky");
		case 80: return QString("Warm Glowing Nightscape");
		case 81: return QString("Artistic Nightscape");
		case 82: return QString("Glittering Illuminations");
		case 83: return QString("Clear Night Portrait");
		case 84: return QString("Soft Image of a Flower");
		case 85: return QString("Appetizing Food");
		case 86: return QString("Cute Desert");
		case 87: return QString("Freeze Animal Motion");
		case 88: return QString("Clear Sports Shot");
		case 89: return QString("Monochrome");
		case 90: return QString("Creative Control");
		case 92: return QString("Handheld Night Shot");
	}
	return QString("Unknown (%1)").arg(code);
}

QString MetadataReader::decodePanasonicAdvancedSceneMode(int shootingMode, int advancedSceneType)
{
	if (shootingMode == 0 && advancedSceneType == 1) return "Off";
	
	if (shootingMode == 2 && advancedSceneType == 2) return "Outdoor Portrait";
	if (shootingMode == 2 && advancedSceneType == 3) return "Indoor Portrait";
	if (shootingMode == 2 && advancedSceneType == 4) return "Creative Portrait";
	
	if (shootingMode == 3 && advancedSceneType == 2) return "Nature";
	if (shootingMode == 3 && advancedSceneType == 3) return "Architecture";
	if (shootingMode == 3 && advancedSceneType == 4) return "Creative Scenery";
	
	if (shootingMode == 4 && advancedSceneType == 2) return "Outdoor Sports";
	if (shootingMode == 4 && advancedSceneType == 3) return "Indoor Sports";
	if (shootingMode == 4 && advancedSceneType == 4) return "Creative Sports";

	if (shootingMode == 9 && advancedSceneType == 2) return "Flower";
	if (shootingMode == 9 && advancedSceneType == 3) return "Objects";
	if (shootingMode == 9 && advancedSceneType == 4) return "Creative Macro";

	if (shootingMode == 21 && advancedSceneType == 2) return "Illuminations";
	if (shootingMode == 21 && advancedSceneType == 4) return "Creative Night Scenery";

	if (shootingMode == 45 && advancedSceneType == 2) return "Cinema";
	if (shootingMode == 45 && advancedSceneType == 7) return "Expressive";
	if (shootingMode == 45 && advancedSceneType == 8) return "Retro";
	if (shootingMode == 45 && advancedSceneType == 9) return "Pure";
	if (shootingMode == 45 && advancedSceneType == 10) return "Elegant";
	if (shootingMode == 45 && advancedSceneType == 12) return "Monochrome";
	if (shootingMode == 45 && advancedSceneType == 13) return "Dynamic Art";
	if (shootingMode == 45 && advancedSceneType == 14) return "Silhouette";

	if (shootingMode == 51 && advancedSceneType == 2) return "HDR Art";
	if (shootingMode == 51 && advancedSceneType == 3) return "HDR B&W";

	if (shootingMode == 59 && advancedSceneType == 1) return "Expressive";
	if (shootingMode == 59 && advancedSceneType == 2) return "Retro";
	if (shootingMode == 59 && advancedSceneType == 3) return "High Key";
	if (shootingMode == 59 && advancedSceneType == 4) return "Sepia";
	if (shootingMode == 59 && advancedSceneType == 5) return "High Dynamic";
	if (shootingMode == 59 && advancedSceneType == 6) return "Miniature";
	if (shootingMode == 59 && advancedSceneType == 9) return "Low Key";
	if (shootingMode == 59 && advancedSceneType == 10) return "Toy Effect";
	if (shootingMode == 59 && advancedSceneType == 11) return "Dynamic Monochrome";
	if (shootingMode == 59 && advancedSceneType == 12) return "Soft";

	if (shootingMode == 66 && advancedSceneType == 1) return "Impressive Art";
	if (shootingMode == 66 && advancedSceneType == 2) return "Cross Process";
	if (shootingMode == 66 && advancedSceneType == 3) return "Color Select";
	if (shootingMode == 66 && advancedSceneType == 4) return "Star";

	if (shootingMode == 90 && advancedSceneType == 3) return "Old Days";
	if (shootingMode == 90 && advancedSceneType == 4) return "Sunshine";
	if (shootingMode == 90 && advancedSceneType == 5) return "Bleach Bypass";
	if (shootingMode == 90 && advancedSceneType == 6) return "Toy Pop";
	if (shootingMode == 90 && advancedSceneType == 7) return "Fantasy";
	if (shootingMode == 90 && advancedSceneType == 8) return "Monochrome";
	if (shootingMode == 90 && advancedSceneType == 9) return "Rough Monochrome";
	if (shootingMode == 90 && advancedSceneType == 10) return "Silky Monochrome";

	if (shootingMode == 92 && advancedSceneType == 1) return "Handheld Night Shot";

	//return QString("Unknown (%1, %2)").arg(shootingMode).arg(advancedSceneType);
	return QString();
}

QString MetadataReader::decodePanasonicBurstMode(int code)
{
	switch (code) {
		case 0: return QString("Off");
		case 1: return QString("On");
		case 2: return QString("Auto Exposure Bracketing");
		case 4: return QString("Unlimited");
		case 8: return QString("White Balance Bracketing");
		case 17: return QString("On (With Flash)");
	}
	return QString("Unknown");
}

QString MetadataReader::decodePanasonicSelfTimer(int code)
{
	switch (code) {
		case 1: return QString("Off");
		case 2: return QString("10 s");
		case 3: return QString("2 s");
		case 4: return QString("10 s (3 pictures)");
	}
	return QString("Unknown");
}

QString MetadataReader::decodePanasonicHdr(int code)
{
	switch (code) {
		case 0: return QString("Off");
		case 100: return QString("1 EV");
		case 200: return QString("2 EV");
		case 300: return QString("3 EV");
		case 32868: return QString("1 EV (Auto)");
		case 32968: return QString("2 EV (Auto)");
		case 33068: return QString("3 EV (Auto)");
	}
	return QString("Unknown (%1)").arg(code);
}

QString MetadataReader::decodePanasonicBracketSettings(int code)
{
	switch (code) {
		case 0: return QString("Off");
		case 1: return QString("3 Images (0/-/+)");
		case 2: return QString("3 Images (-/0/+)");
		case 3: return QString("5 Images (0/-/+)");
		case 4: return QString("5 Images (-/0/+)");
		case 5: return QString("7 Images (0/-/+)");
		case 6: return QString("7 Images (-/0/+)");
	}
	return QString("Unknown (%1)").arg(code);
}

QString MetadataReader::decodeImageCompression(int code)
{
	switch (code) {
		case 1: return QString("Uncompressed");
		case 2: return QString("CCITT 1D");
		case 3: return QString("T4/Group 3 Fax");
		case 4: return QString("T6/Group 4 Fax");
		case 5: return QString("LZW");
		case 6: return QString("JPEG (old-style)");
		case 7: return QString("JPEG");
		case 8: return QString("Adobe Deflate");
		case 9: return QString("JBIG B&W");
		case 10: return QString("JBIG Color");
		case 99: return QString("JPEG");
		case 32766: return QString("Next");
		case 32767: return QString("Sony ARW Compressed");
		case 32769: return QString("Packed RAW");
		case 32770: return QString("Samsung SRW Compressed");
		case 32771: return QString("CCIRLEW");
		case 32772: return QString("Samsung SRW Compressed 2");
		case 32773: return QString("PackBits");
		case 32809: return QString("Thunderscan");
		case 32867: return QString("Kodak KDC Compressed");
		case 32895: return QString("IT8CTPAD");
		case 32896: return QString("IT8LW");
		case 32897: return QString("IT8MP");
		case 32898: return QString("IT8BL");
		case 32908: return QString("PixarFilm");
		case 32909: return QString("PixarLog");
		case 32946: return QString("Deflate");
		case 32947: return QString("DCS");
		case 33003: return QString("Aperio JPEG 2000 YCbCr");
		case 33005: return QString("Aperio JPEG 2000 RGB");
		case 34661: return QString("JBIG");
		case 34676: return QString("SGILog");
		case 34677: return QString("SGILog24");
		case 34712: return QString("JPEG 2000");
		case 34713: return QString("Nikon NEF Compressed");
		case 34715: return QString("JBIG2 TIFF FX");
		case 34718: return QString("Microsoft Document Imaging (MDI) Binary Level Codec");
		case 34719: return QString("Microsoft Document Imaging (MDI) Progressive Transform Codec");
		case 34720: return QString("Microsoft Document Imaging (MDI) Vector");
		case 34887: return QString("ESRI Lerc");
		case 34892: return QString("Lossy JPEG");
		case 34925: return QString("LZMA2");
		case 34926: return QString("Zstd (old)");
		case 34927: return QString("WebP (old)");
		case 34933: return QString("PNG");
		case 34934: return QString("JPEG XR");
		case 50000: return QString("Zstd");
		case 50001: return QString("WebP");
		case 50002: return QString("JPEG XL (old)");
		case 52546: return QString("JPEG XL");
		case 65000: return QString("Kodak DCR Compressed");
		case 65535: return QString("Pentax PEF Compressed");
	}
	return QString("Unknown (%1)").arg(code);
}
