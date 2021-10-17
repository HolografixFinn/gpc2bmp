#include<cstdint>
#include<string>
#include<vector>
#include<exception>
#ifndef __GPC__
#define __GPC__
namespace FairyTale {
	class GPC {

	public:
		GPC();
		void loadFile(const std::string &filename);
		uint16_t getWidth();
		uint16_t getHeight();
		uint16_t getXOffs();
		uint16_t getYOffs();

		// I know, I know. Scott Meyers wouldn't approve returning const refs to members
		const std::vector<uint8_t> &getPalette();
		const std::vector<uint8_t> &getImageData();

	private:
		void loadPaletteFromBuffer();
		void loadInfoFromBuffer();
		void decodeImage();
		void selfXorRow(uint8_t *row, uint16_t max);
		void xorWithPreviousRow(uint8_t *row, uint8_t *prevRow, uint16_t max);
		void planar2chunky(uint8_t *row, uint16_t planeBytesPerRow, uint8_t *chunkyRow);
		inline uint8_t convertColorComponent(uint8_t component);

		std::vector<uint8_t> fileBuffer;
		std::vector<uint8_t> fakeVRAM;
		std::vector<uint8_t> palette;
		std::vector<uint8_t> imageBuffer;
//		unsigned char tempRowBuffer[640];
//		unsigned char lastRowBuffer[640]; //dimensione in eccesso
		uint16_t width;
		uint16_t height;
		uint16_t xOffs;
		uint16_t yOffs;

		uint32_t infoOffs;
		uint32_t dataOffs;
		uint16_t compressedSize;
		uint16_t vramBytesPerRow;
		uint16_t bufferBytesPerRow;
		uint16_t rowsInterleaving;
//		int16_t numCycles;
//		uint32_t interleavedRowsStride;
//		uint32_t startingOffset;

	};
}
class InvalidGpcException : public std::exception {
	const char* what() const noexcept override {
		return "Invalid GPC file";
	}
};
#endif