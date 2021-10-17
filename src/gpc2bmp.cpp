// gpc2bmp.cpp: definisce il punto di ingresso dell'applicazione.
//

#include "gpc2bmp.h"
#include <iostream>
#include "inc/GPC.h"
#include "utils/inc/BmpHelper.h"

using namespace std;

int main(int argc, char * argv[])
{
	cout << "GPC2BMP v" << +GPC2BMP_V_MAJ << "." << +GPC2BMP_V_MIN << std::endl;
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " infile.gpc [outfilename.bmp]" << std::endl;
		return -1;
	}
	std::string outfile;
	if (argc > 2) {
		outfile.assign(argv[2]);
	}
	else {
		outfile.assign(argv[1]);
		outfile += ".bmp";
	}
	try {
		FairyTale::GPC gpc;
		gpc.loadFile(argv[1]);
		BmpHelper::Bmp bmp;
		bmp.setBpp(BmpHelper::Bpp::Bpp8);
		bmp.setCompression(BmpHelper::BmpCompression::BI_RGB);
		bmp.setSize(640, 400);
		bmp.setPalette(gpc.getPalette().data(), 16, 0);
		bmp.setImageData(gpc.getImageData().data());
		bmp.save(outfile);
		cout << "Done converting " << argv[1] << " to " << outfile << std::endl;
	}
	catch (const std::exception &e) {
		cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
