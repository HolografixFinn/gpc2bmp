#include<vector>

#ifndef __BMPHELPER__
#define __BMPHELPER__
namespace BmpHelper {
	// TODO implement other compressions
	enum class BmpCompression : uint32_t { BI_RGB = 0 };
	// TODO implement different bpp versions
	enum class Bpp : uint16_t { Bpp8 = 8 };
	class Bmp {
	public:
		Bmp();
		void setSize(int32_t w, int32_t h);
		void setCompression(BmpCompression compression);
		void setBpp(Bpp bpp);
		bool setPalette(const uint8_t *rgbdata, int count, int startIdx);
		bool setImageData(const uint8_t *imgData);
		void save(const std::string &filename);

	private:
		std::vector<uint8_t> info;
		std::vector<uint8_t> data;
		uint8_t *fileheader;
		uint8_t *infoheader;
		uint8_t *palette;
		int32_t w;
		int32_t h;
		BmpCompression comp;
		Bpp bpp;
		uint32_t numColors;
		uint32_t infoSize;
		uint32_t dataSize;
		uint16_t bytesPerRow;
		void writeW(int32_t);
		void writeH(int32_t);
		void writeComp(BmpCompression c);
		void writeBpp(Bpp b);
		void recalcAll();
	};
}
#endif