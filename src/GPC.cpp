#include"inc/GPC.h"
#include"utils/inc/DataHelper.h"
#include<iostream>
#include<iomanip>
#include<fstream>
namespace FairyTale {
	GPC::GPC() : 
		fileBuffer(0), fakeVRAM(0x8000 * 4, 0xff), palette(0x10*3, 0), imageBuffer(640*400, 0),
		rowsInterleaving(0), infoOffs(0), dataOffs(0) 
	{
	}
	void GPC::loadFile(const std::string &filename) {
		fileBuffer = DataHelper::file2vector<uint8_t>(filename);
		if (fileBuffer.size() < 0x10) {
			throw InvalidGpcException();
		}
		// check if the first 16 bytes represent the "PC98)GPCFILE   " ascii string
		if (std::memcmp(&fileBuffer[0], "\x50\x43\x39\x38\x29\x47\x50\x43\x46\x49\x4c\x45\x20\x20\x20\x00", 0x10) != 0) {
			throw InvalidGpcException();
		}
		this->fakeVRAM.assign(0x8000 * 4, 0xff);
		loadPaletteFromBuffer();
		loadInfoFromBuffer();
		decodeImage();
	}
	void GPC::selfXorRow(uint8_t *row, uint16_t max) {
		if (row[0] != 0) {
			uint8_t interleave = row[0];
			uint8_t lastValue = row[1];
			uint16_t offs = 1+interleave;
			for (uint8_t i = 0; i < interleave; i++) {
				while (offs < max) {
					row[offs] ^= lastValue;
					lastValue = row[offs];
					offs += interleave;
				}
				offs = 1 + i +1;
			}
		}
	}
	
	void GPC::xorWithPreviousRow(uint8_t *row, uint8_t *prevRow, uint16_t max) {
		
		
		uint32_t *r = reinterpret_cast<uint32_t *>(&row[1]);
		uint32_t *p = reinterpret_cast<uint32_t *>(&prevRow[1]);
		max = (max - 1) / 4;
		for (int i = 0; i < max; i++) {
			*r++ ^= *p++;
		}
	}

	void GPC::planar2chunky(uint8_t *row, uint16_t planeBytesPerRow, uint8_t *chunkyRow) {
		// lazy implementation, but it works just fine
		uint8_t *planeA = row + 1;
		uint8_t *planeB = planeA + planeBytesPerRow;
		uint8_t *planeC = planeB + planeBytesPerRow;
		uint8_t *planeD = planeC + planeBytesPerRow;
		for (uint16_t i = 0; i < planeBytesPerRow; ++i) {
			uint8_t byteA = *planeA++;
			uint8_t byteB = *planeB++;
			uint8_t byteC = *planeC++;
			uint8_t byteD = *planeD++;
			for (uint8_t j = 0; j < 8; ++j) {
				uint8_t pixel = ((byteA & 0x80) >> 7) + ((byteB & 0x80) >> 6) + ((byteC & 0x80) >> 5) + ((byteD & 0x80) >> 4);
				*chunkyRow = pixel;
				chunkyRow++;
				byteA <<= 1;
				byteB <<= 1;
				byteC <<= 1;
				byteD <<= 1;
			}
		}
	}

	void GPC::decodeImage() {
		uint16_t linesToDo = this->height;
		uint16_t offs = this->dataOffs;
		uint16_t vramPlaneBytesPerRow = (this->width + 7) >> 3; //each plane has 1 bit per pixel
		uint16_t vramBytesPerRow = (vramPlaneBytesPerRow * 4) + 1; // 4 planes, plus 1 byte needed for row's xor-decryption
		uint32_t decompressedSize = vramBytesPerRow * this->height;
		std::vector<uint8_t> decompBuffer(decompressedSize, 0);
		// first, we decompress all the data
		// after that we take care of xor-decryptionm, rows interleaving and planar-to-chunky conversion
		uint32_t compOffs = 0, decompOffs = 0;
		uint8_t *compressedData = &this->fileBuffer[this->dataOffs];
		while (decompOffs < decompressedSize) {
			uint8_t ctrlByte = compressedData[compOffs++];
			for (uint8_t ctrlBit_counter = 0; ctrlBit_counter < 8 && decompOffs < decompressedSize; ctrlBit_counter++) {
				uint8_t ctrlBit = ctrlByte & 0x80;
				ctrlByte <<= 1;
				if (ctrlBit == 0) {
					std::memset(&decompBuffer[decompOffs], 0, 8);
					decompOffs += 8;
				}
				else {
					uint8_t cmdByte = compressedData[compOffs++];
					for (uint8_t cmdBit_counter = 0; cmdBit_counter < 8 && decompOffs < decompressedSize; cmdBit_counter++) {
						uint8_t cmdBit = cmdByte & 0x80;
						cmdByte <<= 1;
						if (cmdBit == 0) {
							decompBuffer[decompOffs++] = 0;
						}
						else {
							decompBuffer[decompOffs++] = compressedData[compOffs++];
						}
					}
				}
			}
		}
		std::vector<uint8_t> chunkyRow(vramPlaneBytesPerRow * 8, 0);
		uint8_t *row = &decompBuffer[0];
		uint8_t interleaveCycle = 0;
		uint16_t rowPosition = this->yOffs + interleaveCycle;
		for (uint16_t i = 0; i < this->height; i++) {
			// row decrypting
			this->selfXorRow(row, vramBytesPerRow);
			// xoring row with previous row
			if (i > 0) {
				this->xorWithPreviousRow(row, row - vramBytesPerRow, vramBytesPerRow);
			}
			// converting planar 4bpp row data to chunky 8bpp
			planar2chunky(row, vramPlaneBytesPerRow, &chunkyRow[0]);
			// chunky row data -> main image
			uint32_t destOffs = (rowPosition * 640) + xOffs;
			std::memcpy(&imageBuffer[destOffs], &chunkyRow[0], this->width);
			rowPosition += this->rowsInterleaving;
			if (rowPosition >= this->yOffs+this->height) {
				interleaveCycle++;
				rowPosition = this->yOffs + interleaveCycle;
			}
			row += vramBytesPerRow;
		}
		return;
	}
	void GPC::loadInfoFromBuffer() {
		if (fileBuffer.size() == 0) {
			return;
		}
		this->rowsInterleaving = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[0x10]);
		this->infoOffs = DataHelper::readFromPtr_LE<uint32_t>(&fileBuffer[0x18]);
		this->dataOffs = this->infoOffs + 0x10;
		this->width = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[infoOffs]);
		this->height = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[infoOffs+2]);
		this->compressedSize = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[infoOffs+4])-0x10;
		this->xOffs = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[infoOffs + 10]);
		this->yOffs = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[infoOffs + 12]);
		this->vramBytesPerRow = (this->width + 7) >> 3;
		this->bufferBytesPerRow = (this->vramBytesPerRow * 4) + 1;

	}
	void GPC::loadPaletteFromBuffer() {
		if (fileBuffer.size() == 0) {
			return;
		}
		std::fill(palette.begin(), palette.end(), 0);
		uint32_t paletteInfoOffset = DataHelper::readFromPtr_LE<uint32_t>(&fileBuffer[0x14]);
		uint16_t numColors = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[paletteInfoOffset]);
		uint16_t bytesPerColor = DataHelper::readFromPtr_LE<uint16_t>(&fileBuffer[paletteInfoOffset+2]);
		//just out of curiosity
		// assert(numColors == 0x10);
		// assert(bytesPerColor == 2);
		uint16_t *paletteDataPtr = reinterpret_cast<uint16_t *>(&this->fileBuffer[paletteInfoOffset+4]);
		for (int i = 0; i < numColors; i++) {
			uint16_t val = DataHelper::readFromPtr_LE<uint16_t>(paletteDataPtr++);
			uint8_t b = val & 0x0f;
			uint8_t r = (val >> 4) & 0x0f;
			uint8_t g = (val >> 8) & 0x0f;
			r = convertColorComponent(r);
			g = convertColorComponent(g);
			b = convertColorComponent(b);
			this->palette[(i * 3) + 0] = b;
			this->palette[(i * 3) + 1] = g;
			this->palette[(i * 3) + 2] = r;
		}
	}
	inline uint8_t GPC::convertColorComponent(uint8_t component) {
		return (component * 255) / 0x0f;
	}
	uint16_t GPC::getWidth() { return this->width; }
	uint16_t GPC::getHeight() { return this->height; }
	uint16_t GPC::getXOffs() { return this->xOffs; }
	uint16_t GPC::getYOffs() { return this->yOffs; }

	// I know, I know. Scott Meyers wouldn't approve returning const refs to members
	const std::vector<uint8_t> &GPC::getPalette() { return this->palette; }
	const std::vector<uint8_t> &GPC::getImageData() { return this->imageBuffer; }

}
