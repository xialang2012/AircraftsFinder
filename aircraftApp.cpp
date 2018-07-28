#include "aircraftDetection.h"

int main(int argc, char*argv[])
{
	CPLSetConfigOption("GDAL_DATA", "C:\\warmerda\\bld\\data"); // 设定GDAL 空间参考data目录

	GenTypes *genTypes = new GenTypes();
	HandleInput handleInput(genTypes);
	if (handleInput.parseArguments(argc, argv) == -1) return -1;

	FilePathTools pathTool;
	HandleShpFile shpHandle(handleInput.outPath() + "/aircrafts.shp");

	std::string inFile;
	std::vector<std::string> inFiles;

	pathTool.GetFilesFromDir(handleInput.inPath138(), inFiles, "TIF");
	
	CreateLineShp cLineShp(handleInput.outPath() + "/aircrafts_Border.shp");

	// tmp
	std::ofstream outf;
	outf.open("resultFile.txt");

	int totalFile = inFiles.size();
	for (int fileNum = 0; fileNum < totalFile; ++fileNum)
	{
		inFile = inFiles[fileNum];
		//if (fileNum > 30) continue;
		/*test*/
		//std::string in21File = handleInput.inPath21() + "/" + pathTool.GetFileNameNoSuiffix(inFile).substr(0, pathTool.GetFileNameNoSuiffix(inFile).length() - 1) + "7.tif";
		//if (_access(in21File.c_str(), 0) == -1) { std::cout << "h" << std::endl; continue; }
		/*test*/
		
		//if (pathTool.GetFileName(inFile) != "LC80601072017101LGN00_B9.TIF")continue;
		std::cout << "process: " << pathTool.GetFileName(inFile) << ", " << fileNum + 1 << " of " << totalFile << std::endl;
		processOneSwath(outf, inFile, cLineShp, shpHandle, pathTool, *genTypes, handleInput.outPath(), handleInput.inPath21(), handleInput.inPathVapor());
		
	}
	cLineShp.closeShp();
	shpHandle.closeShp();
	outf.close();

	system("pause");

}