#include "aircraftDetection.h"

int main(int argc, char*argv[])
{
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
		std::cout << "process: " << pathTool.GetFileName(inFile) << ", " << fileNum + 1 << " of " << totalFile << std::endl;
		processOneSwath(outf, inFile, cLineShp, shpHandle, pathTool, *genTypes, handleInput.outPath(), handleInput.inPath21(), handleInput.inPathVapor());
		
	}
	cLineShp.closeShp();
	shpHandle.closeShp();
	outf.close();
}