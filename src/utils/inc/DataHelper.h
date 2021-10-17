#include<cstdint>
#include<array>
#include<string>
#include<fstream>
#include<vector>
#include<cassert>
#ifndef  __DATAHELPER__
#define __DATAHELPER__

class DataHelper {
public:
	static const bool isBigEndian;
	template<typename C> static C readFromPtr_LE(const void *ptr);
	template<typename C> static C readFromPtr_BE(const void *ptr);
	template<typename C> static void writeToPtr_LE(void *ptr,C val);
	template<typename C> static void writeToPtr_BE(void *ptr,C val);
	template<typename C> static void vector2file(const std::vector<C> &v, int size, std::string filename);
	template<typename C> static std::vector<C> file2vector(std::string filename);
	template<typename C> static std::vector<C> extractFilePart(std::string filename, uint32_t start, uint32_t len);
	static bool checkFilesEquality(std::string f1, std::string f2);
	static uint64_t getFileSize(std::string filename);
private:
	static bool fIsBigEndian();
	template <typename T> static void SwapEndian(T &val);

};
template <typename T>
void DataHelper::SwapEndian(T &val) {
	union U {
		T val;
		std::array<std::uint8_t, sizeof(T)> raw;
	} src, dst;

	src.val = val;
	std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
	val = dst.val;
}
template<typename C> C DataHelper::readFromPtr_LE(const void *ptr) {
	C val = *static_cast<const C *>(ptr);
	if (DataHelper::isBigEndian) {
		DataHelper::SwapEndian(val);
	}
	return val;
}
template<typename C> C DataHelper::readFromPtr_BE(const void *ptr) {
	C val = *reinterpret_cast<const C *>(ptr);
	if (!DataHelper::isBigEndian) {
		DataHelper::SwapEndian(val);
	}
	return val;
}
template<typename C> void DataHelper::writeToPtr_LE(void *ptr, C val) {
	C *ptr2 = static_cast<C *>(ptr);
	if (DataHelper::isBigEndian) {
		DataHelper::SwapEndian(val);
	}
	*ptr2 = val;
}
template<typename C> void DataHelper::writeToPtr_BE(void *ptr, C val) {
	C *ptr2 = reinterpret_cast<C *>(ptr);
	if (!DataHelper::isBigEndian) {
		DataHelper::SwapEndian(val);
	}
	*ptr2 = val;
}


template<> uint8_t DataHelper::readFromPtr_LE<uint8_t>(const void *ptr);
template<> uint8_t DataHelper::readFromPtr_BE<uint8_t>(const void *ptr);

template<> void DataHelper::writeToPtr_LE<uint8_t>(void *ptr,uint8_t val);
template<> void DataHelper::writeToPtr_BE<uint8_t>(void *ptr,uint8_t val);

template<typename C> static void DataHelper::vector2file(const std::vector<C> &v, int size, std::string filename) {
	std::ofstream f(filename, std::ios::out|std::ios::binary);
	f.write(reinterpret_cast<const char *>(&v[0]), size * sizeof(C));
	f.close();
}
template<typename C> static std::vector<C> DataHelper::file2vector(std::string filename) {
	std::ifstream f(filename, std::ios::in|std::ios::binary);
	if (!f.is_open()) {
		throw std::exception("Can't open file");
	}
	// sooner or later I'm gonna fix this: according to the standard, tellg() is not required to return the file position in bytes,
	// https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file
	auto size = f.seekg(0, std::ios::end).tellg();
	f.seekg(0, std::ios::beg);
	std::vector<C> v(size / sizeof(C), 0);
	f.read(reinterpret_cast<char *>(&v[0]), size);
	f.close();
	return v;
}
template<typename C> static std::vector<C> DataHelper::extractFilePart(std::string filename, uint32_t start, uint32_t len) {
	std::ifstream f(filename, std::ios::in | std::ios::binary);
	f.seekg(start, std::ios::beg);
	std::vector<C> v(len / sizeof(C), 0);
	f.read(reinterpret_cast<char *>(&v[0]), len);
	f.close();
	return v;
}

#endif // ! __DATAHELPER__
