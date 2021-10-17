#include"inc/DataHelper.h"
// #include<fstream>
bool DataHelper::fIsBigEndian() {
	union {
		uint16_t i;
		uint8_t c[2];
	} u;
	u.i = 1;
	return u.c[1] == 1;
}

const bool DataHelper::isBigEndian = DataHelper::fIsBigEndian();

template<> uint8_t DataHelper::readFromPtr_LE<uint8_t>(const void *ptr) {
	return *reinterpret_cast<const uint8_t *>(ptr);
}

template<> uint8_t DataHelper::readFromPtr_BE<uint8_t>(const void *ptr) {
	return *reinterpret_cast<const uint8_t *>(ptr);
}

template<> void DataHelper::writeToPtr_LE<uint8_t>(void *ptr, uint8_t val) {
	*reinterpret_cast<uint8_t *>(ptr) = val;
}
template<> void DataHelper::writeToPtr_BE<uint8_t>(void *ptr, uint8_t val) {
	*reinterpret_cast<uint8_t *>(ptr) = val;
}
bool DataHelper::checkFilesEquality(std::string f1, std::string f2) {
	std::ifstream file1(f1, std::ios::in | std::ios::binary);
	uint64_t size1=file1.seekg(0, std::ios::end).tellg();
	file1.seekg(0, std::ios::beg);
	std::ifstream file2(f2, std::ios::in | std::ios::binary);
	uint64_t size2 = file2.seekg(0, std::ios::end).tellg();
	file2.seekg(0, std::ios::beg);
	if (size1 != size2) {
		return false;
	}
	std::vector<uint8_t> v1(size1, 0);
	file1.read(reinterpret_cast<char *>(&v1[0]), size1);
	std::vector<uint8_t> v2(size2, 0);
	file2.read(reinterpret_cast<char *>(&v2[0]), size2);
	for (uint64_t i = 0; i < size1; i++) {
		if (v1[i] != v2[i]) {
			return false;
		}
	}
	return true;
}
uint64_t DataHelper::getFileSize(std::string filename) {
	std::ifstream file1(filename, std::ios::in | std::ios::binary);
	uint64_t size1 = file1.seekg(0, std::ios::end).tellg();
	return size1;
}
