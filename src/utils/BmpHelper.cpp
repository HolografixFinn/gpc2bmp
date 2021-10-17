#include"inc/BmpHelper.h"
#include"inc/DataHelper.h"
#include<fstream>

namespace BmpHelper {
	Bmp::Bmp(): info(14+50+(256*4), 0), data(0), fileheader(&info[0]), infoheader(&info[14]), palette(&info[54]),
		w(0), h(0), comp(BmpCompression::BI_RGB), bpp(Bpp::Bpp8), numColors(256), infoSize(14+50+(256*4)), dataSize(0), bytesPerRow(0)
	{
		fileheader[0] = 0x42;
		fileheader[1] = 0x4d;
		DataHelper::writeToPtr_LE<uint32_t>(&infoheader[0], 40);
		DataHelper::writeToPtr_LE<uint16_t>(&infoheader[12], 1);
		DataHelper::writeToPtr_LE<uint16_t>(&infoheader[14], static_cast<typename std::underlying_type<Bpp>::type>(bpp));
		DataHelper::writeToPtr_LE<uint32_t>(&infoheader[16], static_cast<typename std::underlying_type<BmpCompression>::type>(comp));
		DataHelper::writeToPtr_LE<uint32_t>(&infoheader[24], 3779);
		DataHelper::writeToPtr_LE<uint32_t>(&infoheader[28], 3779);
		writeBpp(bpp);

	}
	void Bmp::writeW(int32_t w) {
		if (w < 0) {
			throw std::exception("Invalid negative Width");
		}
		DataHelper::writeToPtr_LE<int32_t>(&infoheader[4], w);
	}
	void Bmp::writeH(int32_t h) {
		DataHelper::writeToPtr_LE<int32_t>(&infoheader[8], h);
	}
	void Bmp::writeComp(BmpCompression c) {
		DataHelper::writeToPtr_LE<uint32_t>(&infoheader[16], static_cast<typename std::underlying_type<BmpCompression>::type>(c));
	}
	void Bmp::writeBpp(Bpp b) {
		DataHelper::writeToPtr_LE<uint16_t>(&infoheader[14], static_cast<typename std::underlying_type<Bpp>::type>(b));
		switch (b) {
		case Bpp::Bpp8:
			numColors = 256;
			break;
		default:
			throw std::exception("Unimplemented Bpp");
			break;
		}
		DataHelper::writeToPtr_LE<uint32_t>(&infoheader[32], numColors);
	}
	void Bmp::recalcAll() {
		infoSize = 14 + 40;
		switch (bpp) {
		case Bpp::Bpp8:
			infoSize += 256 * 4;
			bytesPerRow = w;
			break;
		default:
			throw std::exception("Unimplemented Bpp");
			break;
		}
		bytesPerRow = (bytesPerRow + 3) & 0xfffc;
		dataSize = bytesPerRow * std::abs(h);
		data.resize(dataSize);
		uint32_t totalSize = infoSize + dataSize;

		DataHelper::writeToPtr_LE<uint32_t>(&fileheader[2], totalSize);
		DataHelper::writeToPtr_LE<uint32_t>(&fileheader[10], infoSize);
		DataHelper::writeToPtr_LE<uint32_t>(&infoheader[20], dataSize);
	}
	void Bmp::setSize(int32_t _w, int32_t _h) {
		writeW(_w);
		writeH(_h);
		this->w = _w;
		this->h = _h;
		recalcAll();
	}
	void Bmp::setCompression(BmpCompression compression) {
		writeComp(compression);
		this->comp = compression;
		recalcAll();
	}
	void Bmp::setBpp(Bpp _bpp) {
		writeBpp(_bpp);
		this->bpp = _bpp;
		recalcAll();
	}
	bool Bmp::setPalette(const uint8_t *rgbdata, int count, int startIdx) {
		uint16_t max = startIdx + count;
		if (max >= numColors) {
			throw std::exception("Invalid palette indices");
		}
		uint8_t *dest = &palette[startIdx * 4];
		for (uint16_t i = 0; i < count; i++) {
			dest[0] = *rgbdata++;
			dest[1] = *rgbdata++;
			dest[2] = *rgbdata++;
			dest[3] = 0;
			dest += 4;
		}
		return true;
	}
	bool Bmp::setImageData(const uint8_t *imgData) {
		const uint8_t *start;
		uint8_t *dest = &data[0];
		int32_t step;
		uint32_t size;
		switch (bpp) {
		case Bpp::Bpp8:
			size = w;
			if (h >= 0) {
				// bottom-up
				start = imgData + (w*h) - w;
				step = -w;
				
			}
			else {
				start = imgData;
				step = w;
			}
			for (uint16_t i = 0; i < abs(h); i++) {
				std::memcpy(dest, start, size);
				start += step;
				dest += bytesPerRow;
			}
			break;
		default:
			throw std::exception("Unimplemented bpp");
			break;
		}
		return true;
	}
	void Bmp::save(const std::string &filename) {
		std::ofstream f(filename, std::ios::binary | std::ios::out);
		f.write(reinterpret_cast<const char *>(&info[0]), infoSize);
		f.write(reinterpret_cast<const char *>(&data[0]), dataSize);
	}
}