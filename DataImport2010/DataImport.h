#ifndef DATAIMPORT_H
#define DATAIMPORT_H

#include <QtXml> 
#include <QDir>
#include "OERSProductLayer.h"

using namespace std;

class DataImport
{
private:
	QString sourceDirec;
	QString targetDirec;

public:
	void Import();
	static QString dbTable;

private:
	void SetDirec();
	vector<QString> MoveFiles();
	void ImportFiles(QFileInfoList fileFullInfo);
	void execDataImp(QString fileName, bool IsExistDB);
	void getProductProperties(OERSProductProperties& prop, char** papszMetadata);
	bool importSSW(GDALDataset *pDataset, OERSProductProperties prop, bool IsExistDB);
	bool makeTiff(GDALDataset *pDataset, OERSProductProperties prop, bool IsExist);
	bool CreateSSWTable(string fileName);
	bool AddRecord(OERSProductProperties prop, bool IsExist);
	bool createLZWCompressfile(string fileName, string lzwfileName);
	bool createZIPCompressfile(string& dirpathName, string& zipfileName, string parentdirName);

};

#endif