#include <vector>
#include <iostream>
#include <string>
#include <fstream>

#include <ogr_spatialref.h>
#include <io.h>  
#include<windows.h>

#include "zip.h"


#include <QString>

#include "DataImport.h"
#include "DataImportUtils.h"

using namespace std;

QTime _time = QTime::fromString("000000", "hhmmss");

QString DataImport::dbTable = "";

bool DelDir(const QString &path)
{
	if (path.isEmpty()){
		return false;
	}
	QDir dir(path);
	if (!dir.exists()){
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤  
	QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息  
	foreach(QFileInfo file, fileList){ //遍历文件信息  
		if (file.isFile()){ // 是文件，删除  
			file.dir().remove(file.fileName());
		}
		else{ // 递归删除  
			DelDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath()); // 删除文件夹  
}

int _System(const char * cmd, char *pRetMsg, int msg_len)
{
	FILE * fp;
	char * p = NULL;
	int res = -1;
	if (cmd == NULL || pRetMsg == NULL || msg_len < 0)
	{
		printf("Param Error!\n");
		return -1;
	}
	if ((fp = _popen(cmd, "r")) == NULL)
	{
		printf("Popen Error!\n");
		return -2;
	}
	else
	{
		memset(pRetMsg, 0, msg_len);
		//get lastest result    
		while (fgets(pRetMsg, msg_len, fp) != NULL)
		{
			printf("Msg:%s", pRetMsg); //print all info    
		}

		if ((res = _pclose(fp)) == -1)
		{
			printf("close popenerror!\n");
			return -3;
		}
		pRetMsg[strlen(pRetMsg) - 1] = '\0';
		return 0;
	}
}

QFileInfoList GetFileList(QString path)
{
	QDir dir(path);
	QStringList nameFilters;
	nameFilters << "*.hdf";
	QFileInfoList file_list = dir.entryInfoList(nameFilters,QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

	for(int i = 0; i != folder_list.size(); i++)
	{
		QString name = folder_list.at(i).absoluteFilePath();
		QFileInfoList child_file_list = GetFileList(name);
		file_list.append(child_file_list);
	}

	return file_list;
}

QFileInfoList GetFileListFromTxt()
{
	ifstream infile; 
	infile.open("SourceTiff_file2_path.txt");   //将文件流对象与文件连接起来 

	QFileInfoList file_list;
	string sstFile;
	while(getline(infile,sstFile))
	{
		//if (QString::fromLocal8Bit(sstFile.c_str()).contains("NOAA_NOAA_AVHRR"))
		{
			QFileInfo fileTemp(QString::fromLocal8Bit(sstFile.c_str()));
			file_list.append(fileTemp);
		}
	}
	infile.close();
	return file_list;
}

void DataImport::Import()
{
	SetDirec();
	QFileInfoList fileFullInfo=GetFileList(this->sourceDirec);
	//QDebug() << QFileInfoList::fileFullInfo << endl;
	//QFileInfoList fileFullInfo=GetFileListFromTxt();

	ImportFiles(fileFullInfo);
}

void DataImport::SetDirec()
{
	this->targetDirec = DataImportUtils::getSettingByNodePath("/paramSetting/TargetDir");
	this->sourceDirec = DataImportUtils::getSettingByNodePath("/paramSetting/SourceDir");
}

vector<QString> DataImport::MoveFiles()
{
	vector<QString> fnames;
	QString pFileName;
	QString fileTarPath = "";
	QString fileSouPath = "";

	QDir dir(this->sourceDirec);
	QStringList nameFilters;
	nameFilters << "*.hdf";
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

	if (files.count() > 0)
	{
		int maxIndex = 50;
		if (files.count() < 50)
			maxIndex = files.count();
		for (int i = 0; i < maxIndex; i++)
		{
			pFileName = files[i];

			//自动进行数据的覆盖，此处不检查，后面直接覆盖！
			fileTarPath = this->targetDirec + "\\" + files[i];
			fileSouPath = this->sourceDirec + "\\" + files[i];
			QFile sfile(fileTarPath);
			if (sfile.exists())
			{
				QFile::remove(fileTarPath);
			}
			QFile::copy(fileSouPath, fileTarPath);
			QFile::remove(fileSouPath);
			fnames.push_back(pFileName);
		}
	}
	return fnames;
}

//循环文件入库
void DataImport::ImportFiles(QFileInfoList fileFullInfo)
{
	for (int i=0;i<fileFullInfo.size();i++)
	{
		QString targetFile=this->targetDirec + "\\" + fileFullInfo.at(i).fileName();
		QString targetFileXML=targetFile+".aux.xml";

		QFile sfile(targetFile);
		QFile sfile2(targetFileXML);

		bool IsExistDB = false;
		QString fileName=fileFullInfo.at(i).fileName();
		//string sql_query = "select id from raster.chn_layer where name='" + fileName.mid(0,fileName.length()-4).toStdString() + "'";
		string sql_query = "select id from raster." + DataImport::dbTable.toStdString() + " where name='" + fileName.mid(0, fileName.length() - 4).toStdString() + "'";

		QSqlDatabase dbRaster = QSqlDatabase::database();
		dbRaster = QSqlDatabase::addDatabase("QPSQL");
		dbRaster.setDatabaseName(DataImportUtils::getSettingByNodePath("/paramSetting/database"));
		dbRaster.setHostName(DataImportUtils::getSettingByNodePath("/paramSetting/serverIP"));
		dbRaster.setPort(7359);
		dbRaster.setUserName("postgres");
		dbRaster.setPassword("1q!@hyes0913");
		if (!dbRaster.open())
		{
			qDebug(QString::fromLocal8Bit("raster db open failed.").toStdString().c_str());
			return;
		}

		//if (!DataImportUtils::db.isOpen())
		//{
		//	DataImportUtils::db.open();
		//}
		QSqlQuery query(dbRaster);//(DataImportUtils::db);
		query.exec(sql_query.c_str());
		while (query.next())
		{
			//string delTable = "drop table vector_ssw." + prop.LayerName;
			//query.exec(delTable.c_str());
			IsExistDB = true;
		}

		dbRaster.close();

		if (IsExistDB)
		//if ((sfile.exists()||sfile2.exists())&&IsExistDB)
		{
			cout<<targetFile.toStdString()<<" exists"<<endl;
			continue;
			//QFile::remove(targetFile);
		}
		cout<<targetFile.toStdString()<<" import"<<endl;
		QFile::remove(targetFile); //防止没有更新覆盖
		QFile::copy(fileFullInfo.at(i).absoluteFilePath(), targetFile);
		execDataImp(fileFullInfo.at(i).fileName(),IsExistDB);
		//QFile::remove(fileFullInfo.at(i).absoluteFilePath());
	}
}

void DataImport::execDataImp(QString fileName,bool IsExistDB)
{
	QString FileFulPath = this->targetDirec + "\\" + fileName;
	try
	{
		qDebug(FileFulPath.toLocal8Bit().toStdString().c_str());

		OERSProductProperties prop;
		fileName = fileName.split(".").at(0);

		QStringList fileNameList = fileName.split("_");
		prop.Facility = fileNameList[0].toStdString();

		prop.ProdSat = fileNameList[1].toStdString();

		prop.SensorType = fileNameList[2].toStdString();
		if (fileNameList[3].contains("TO"))
		{
			QStringList timeList = fileNameList[3].split("TO");
			QDate startDate = QDate::fromString(timeList[0], "yyyyMMdd");
			QDate endDate = QDate::fromString(timeList[1], "yyyyMMdd");
			prop.StartTime = QDateTime(startDate, _time);
			prop.EndTime = QDateTime(endDate, _time);
			prop.ProdHour = 0;
			if (startDate.daysTo(endDate)<6)
			{
				prop.ProdTimeType = 1;
			}
			else if (startDate.daysTo(endDate) == 7)
			{
				prop.ProdTimeType = 7;
			}
			else if (startDate.daysTo(endDate) == 8)
			{
				prop.ProdTimeType = 8;
			}
			else if (startDate.daysTo(endDate) == 10)
			{
				prop.ProdTimeType = 10;
			}
			else if (startDate.daysTo(endDate) >26 && startDate.daysTo(endDate)<32)
			{
				prop.ProdTimeType = 30;
			}
			else if (startDate.daysTo(endDate) > 300)
			{
				prop.ProdTimeType = 365;
				prop.ProdMonth = 0;
			}

		}
		else
		{
			prop.StartTime = QDateTime::fromString(fileNameList[3], "yyyyMMddHHmmss");
			prop.EndTime = QDateTime::fromString(fileNameList[3], "yyyyMMddHHmmss");
			prop.ProdHour = fileNameList[3].mid(8, 2).toInt();
			prop.ProdTimeType = 0;
		}
		if (prop.ProdTimeType != 365)
		{
			prop.ProdMonth = prop.StartTime.date().month();
		}
		prop.ProdLevel = fileNameList[4].toStdString();
		prop.ProductRegion = fileNameList[5].toStdString();
		prop.Resolution = fileNameList[6].toStdString();
		prop.ProdType = fileNameList[7].toStdString();
		prop.ArithmeticType = fileNameList[8].toStdString();
		prop.LayerName = fileName.toStdString();

		GDALAllRegister();
		GDALDataset *pDataset = (GDALDataset*)GDALOpen(FileFulPath.toStdString().c_str(), GA_ReadOnly);
		if (pDataset == NULL)
		{
			qDebug(QString::fromLocal8Bit("hdf文件打开失败.").toStdString().c_str());
			return;
		}
		//获取外层属性
		char** papszMetadata = GDALGetMetadata(pDataset, NULL);

		getProductProperties(prop, papszMetadata);

		////数据库中是否存在
		//bool IsExistDB = false;
		//string sql_query = "select id from raster.chn_layer where name='" + prop.LayerName + "'";
		//if (!DataImportUtils::db.isOpen())
		//{
		//	DataImportUtils::db.open();
		//}
		//QSqlQuery query(DataImportUtils::db);
		//query.exec(sql_query.c_str());
		//while (query.next())
		//{
		//	string delTable = "drop table vector_ssw." + prop.LayerName;
		//	query.exec(delTable.c_str());
		//	IsExistDB = true;
		//}

		if (prop.ProdType == "SSW"||prop.ProdType == "GEOVEL")
		{
			if (!importSSW(pDataset, prop, IsExistDB))
			{
				return;
			}

		}
		else
		{
			if (!makeTiff(pDataset, prop, IsExistDB))
			{
				qDebug(QString::fromLocal8Bit("入库失败.").toStdString().c_str());
				return;
			}
		}

		qDebug(QString::fromLocal8Bit("入库成功.").toStdString().c_str());
		GDALClose(pDataset);
		QFile::remove(FileFulPath);
	}
	catch (...)
	{
		qDebug(QString::fromLocal8Bit("入库失败.").toStdString().c_str());
	}
}

void DataImport::getProductProperties(OERSProductProperties& prop, char** papszMetadata)
{
	QString proStr;
	if (CSLCount(papszMetadata) > 0)
	{
		for (int i = 0; papszMetadata[i] != NULL; i++)
		{
			proStr = QString::fromLocal8Bit(papszMetadata[i]);
			string proStdStr = proStr.toStdString();
			if (proStr.contains("NorthLatitude"))
			{
				prop.ProdLatMax = proStr.split("=").at(1).toDouble();
			}
			else if (proStr.contains("SouthLatitude"))
			{
				prop.ProdLatMin = proStr.split("=").at(1).toDouble();
			}
			else if (proStr.contains("EastLongitude"))
			{
				prop.ProdLonMax = proStr.split("=").at(1).toDouble();
			}
			else if (proStr.contains("WestLongitude"))
			{
				prop.ProdLonMin = proStr.split("=").at(1).toDouble();
			}
			else if (proStr.contains("RowNumber"))
			{
				prop.rnum = proStr.split("=").at(1).toInt();
			}
			else if (proStr.contains("ColNumber"))
			{
				prop.cnum = proStr.split("=").at(1).toInt();
			}
			else if (proStr.contains("LongitudeStep"))
			{
				prop.LongitudeStep = proStr.split("=").at(1).toDouble();
			}
			else if (proStr.contains("LatitudeStep"))
			{
				prop.LatitudeStep = proStr.split("=").at(1).toDouble();
			}
			else if (proStr.contains("Units"))
			{
				prop.Unit = proStr.split("=").at(1).toStdString();
			}
		}
	}
}

//导入风场数据
bool DataImport::importSSW(GDALDataset *pDataset, OERSProductProperties prop, bool IsExistDB)
{
	//获取内层属性
	char ** papszSUBDATASETS = GDALGetMetadata(pDataset, "SUBDATASETS");
	GDALDataset *pDatasetSSWS = NULL;
	GDALDataset *pDatasetSSWD = NULL;
	if (CSLCount(papszSUBDATASETS) > 0)
	{
		for (int i = 0; papszSUBDATASETS[i] != NULL; i++)
		{
			if (i % 2 != 0)
				continue;

			string tmpstr = string(papszSUBDATASETS[i]);
			tmpstr = tmpstr.substr(tmpstr.find_first_of("=") + 1);
			string tmpdsc = string(papszSUBDATASETS[i + 1]);
			tmpdsc = tmpdsc.substr(tmpdsc.find_first_of("=") + 1);

			if (QString::fromLocal8Bit(tmpdsc.c_str()).contains("SSWS")||QString::fromLocal8Bit(tmpdsc.c_str()).contains("GEOVELS"))
			{
				pDatasetSSWS = (GDALDataset*)GDALOpen(tmpstr.c_str(), GA_ReadOnly);
				char** papszMetadata2 = GDALGetMetadata(pDatasetSSWS, NULL);
				getProductProperties(prop, papszMetadata2);
				if (!makeTiff(pDatasetSSWS, prop, IsExistDB))
				{
					qDebug(QString::fromLocal8Bit("生成tif失败.").toStdString().c_str());
					return false;
				}
			}
			else if (QString::fromLocal8Bit(tmpdsc.c_str()).contains("SSWD")||QString::fromLocal8Bit(tmpdsc.c_str()).contains("GEOVELD"))
			{
				pDatasetSSWD = (GDALDataset*)GDALOpen(tmpstr.c_str(), GA_ReadOnly);
			}
		}
	}
	//if (prop.ProdTimeType==1&&prop.StartTime.date().year()<=1991&&prop.StartTime.date().month()<=1)
	//{
		//return true;
	//}
	bool createTable = CreateSSWTable(prop.LayerName);
	if (createTable)
	{
		//风场矢量入库
		int stepForGrid = 8;//格网间距默认为二个格网
		float x, y, ssws, sswd;
		float Threshold = 0.2;

		if (prop.ProdType == "GEOVEL")
		{
			//stepForGrid=6;
			Threshold = 0;
		}

		GDALRasterBand *pRasterBandSSWS = pDatasetSSWS->GetRasterBand(1);
		GDALRasterBand *pRasterBandSSWD = pDatasetSSWD->GetRasterBand(1);

		double min, max, mean, dev;

		pRasterBandSSWS->ComputeStatistics(0, &min, &max, &mean, &dev, NULL, NULL);

		GDALDataType dataType;
		dataType = pRasterBandSSWS->GetRasterDataType();
		float *pdatavalue = new float[1];
		vector<vector<float>> compList;
		string arrayStr;
		vector<float> sswsVector, sswdVector;
		for (int row = 0; row < prop.rnum; row = row + stepForGrid)
		{
			for (int col = 0; col < prop.cnum; col = col + stepForGrid)
			{
				x = prop.ProdLonMin + (float)col * prop.LongitudeStep;
				y = prop.ProdLatMax - (float)row * prop.LatitudeStep;

				pRasterBandSSWS->RasterIO(GF_Read, col, row, 1, 1, pdatavalue, 1, 1, dataType, 0, 0);
				ssws = pdatavalue[0];//风速
				if (ssws > Threshold)
				{
					pRasterBandSSWD->RasterIO(GF_Read, col, row, 1, 1, pdatavalue, 1, 1, dataType, 0, 0);
					sswd = pdatavalue[0];//角度
					sswsVector.push_back(ssws);
					sswdVector.push_back(sswd);

					sswd *= 0.01745329;//pi/180
					vector<float> tempArray;
					tempArray.resize(10);

					//float mainLen = (max - 0.13333333) * 0.075;
					//float mainLen = (ssws - 0.13333333) * 0.075;
					float mainLen=0.4175;//25km

					/*if (prop.ProdType == "GEOVEL")
					{
					mainLen=0.5;
					}*/
					

					float arrLen = mainLen *0.66666667;//2/3长度

					float x1 = x - mainLen * sin(sswd);
					float y1 = y - mainLen * cos(sswd);

					float x2 = x + mainLen * sin(sswd);
					float y2 = y + mainLen * cos(sswd);

					float x3 = x2 - arrLen * sin(sswd + 0.34906585);//20度
					float y3 = y2 - arrLen * cos(sswd + 0.34906585);

					float x4 = x2 - arrLen * sin(sswd - 0.34906585);
					float y4 = y2 - arrLen * cos(sswd - 0.34906585);

					tempArray[0] = sswd;
					tempArray[1] = ssws;
					tempArray[2] = x1;
					tempArray[3] = y1;
					tempArray[4] = x2;
					tempArray[5] = y2;
					tempArray[6] = x3;
					tempArray[7] = y3;
					tempArray[8] = x4;
					tempArray[9] = y4;
					compList.push_back(tempArray);
				}
			}

		}
		QSqlDatabase dbssw = QSqlDatabase::database();
		dbssw = QSqlDatabase::addDatabase("QPSQL");
		dbssw.setDatabaseName(DataImportUtils::getSettingByNodePath("/paramSetting/database"));
		dbssw.setHostName(DataImportUtils::getSettingByNodePath("/paramSetting/vectorIP"));
		dbssw.setPort(7359);
		dbssw.setUserName("postgres");
		dbssw.setPassword("1q!@hyes0913");
		if (!dbssw.open())
		{
			qDebug(QString::fromLocal8Bit("ssw db open failed.").toStdString().c_str());
			return false;
		}
		/*if (!DataImportUtils::db.isOpen())
		{
			DataImportUtils::db.open();
		}*/
		QSqlQuery query(dbssw);
		for (int i = 0; i < sswsVector.size(); i++)
		{
			vector<float> d;
			d.resize(10);

			d = compList[i];
			QString sql_insert = "insert into vector_ssw." + QString::fromLocal8Bit(prop.LayerName.c_str()) + "(SSWD, SSWS, GEOM) values";

			sql_insert += "( " + QString::number(d[0]) + ",";
			sql_insert += QString::number(d[1]) + ",";
			sql_insert += "ST_GeomFromText ('MULTILINESTRING((" + QString::number(d[6]) + " " + QString::number(d[7]) + "," + QString::number(d[4]) + " " + QString::number(d[5]) + "," + QString::number(d[8]) + " " + QString::number(d[9]) + ")" + ",";
			sql_insert += "(" + QString::number(d[4]) + " " + QString::number(d[5]) + "," + QString::number(d[2]) + " " + QString::number(d[3]) + "))'))";
			query.exec(sql_insert);
		}
		dbssw.close();
	}
	else
	{
		qDebug(QString::fromLocal8Bit("createSSWTable失败.").toStdString().c_str());
		return false;
	}
	return true;
}

//创建风场数据库
bool DataImport::CreateSSWTable(string fileName)
{

	QSqlDatabase dbssw = QSqlDatabase::database();
	dbssw = QSqlDatabase::addDatabase("QPSQL");
	dbssw.setDatabaseName(DataImportUtils::getSettingByNodePath("/paramSetting/database"));
	dbssw.setHostName(DataImportUtils::getSettingByNodePath("/paramSetting/vectorIP"));
	dbssw.setPort(7359);
	dbssw.setUserName("postgres");
	dbssw.setPassword("1q!@hyes0913");
	if (!dbssw.open())
	{
		qDebug(QString::fromLocal8Bit("ssw db open failed.").toStdString().c_str());
		return false;
	}
	//if (!DataImportUtils::db.isOpen())
	//{
	//	DataImportUtils::db.open();
	//}
	string count_sql = "select count(*) from pg_class where relname ='" + fileName + "'";
	QSqlQuery query(dbssw);
	query.exec(QString::fromStdString(count_sql));
	query.next();
	if (query.value(0).toInt() > 0)
	{
		string drop_table = "drop table vector_ssw." + fileName;
		query.exec(QString::fromStdString(drop_table));
	}

	string create_sql = "create table vector_ssw." + fileName + "(" + "GID serial primary key,";
	create_sql += "SSWD";
	create_sql += " numeric,";
	create_sql += "SSWS";
	create_sql += " numeric,";
	create_sql += "GEOM geography (multilinestring, 4326))";  //geography (multilinestring, 4326)

	bool isSuccess = query.exec(QString::fromStdString(create_sql));
	dbssw.close();
	return isSuccess;
}

//生成tif并插入表
bool DataImport::makeTiff(GDALDataset *pDataset, OERSProductProperties prop, bool IsExist = false)
{
	double adfGeoTransform[6] = { prop.ProdLonMin, prop.LongitudeStep, 0, prop.ProdLatMax, 0, -prop.LongitudeStep };
	pDataset->SetGeoTransform(adfGeoTransform);
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("Gtiff");
	string lastDir = DataImportUtils::getSettingByNodePath("/paramSetting/TargetTIFFDir").toStdString() + "\\" + prop.ProdType + "\\" + QString::number(prop.ProdTimeType).toStdString();
	prop.Imagehdfspath = "OSGOCEAN/"+prop.ProdType + "/" + QString::number(prop.ProdTimeType).toStdString()+"/"+prop.LayerName + ".tif";;
	string dstPath = lastDir + "\\" + prop.LayerName;
	string dstFile = dstPath + ".tif";
	string dstZIPFile = dstPath + ".zip";
	string fileNameTif=prop.LayerName + ".tif";

	QDir sdir(lastDir.c_str());
	if (!sdir.exists())
	{
		sdir.mkpath(lastDir.c_str());//创建多级目录
	}
	QDir sdir2(dstPath.c_str());
	if (sdir2.exists())
	{
		DelDir(dstPath.c_str());
	}
	QFile sfile(dstZIPFile.c_str());
	if (sfile.exists())
	{

		QFile::remove(dstZIPFile.c_str());
	}

	GDALRasterBand *pRasterBand = pDataset->GetRasterBand(1);
	if (DataImport::dbTable == "gml_chn_layer")
	{
		pRasterBand->SetNoDataValue(-999.0);
	}
	else
	{
		pRasterBand->SetNoDataValue(-9999.0);
	}
	
	double* ignoreValues = new double[2];
	ignoreValues[0] = 1.1754943508222875e-038;
	ignoreValues[1] = -999;
	pRasterBand->SetStatIngoreValues(&ignoreValues, 2);
	double min, max, mean, dev;

	pRasterBand->ComputeStatistics(0, &min, &max, &mean, &dev, NULL, NULL);

	string minstr = QString::number(min).toStdString();
	string maxstr = QString::number(max).toStdString();
	if (minstr == "inf" || minstr == "-inf")
	{
		double* ignoreValues = new double[3];
		ignoreValues[0] = -999;
		ignoreValues[1] = 1.1754943508222875e-038;
		ignoreValues[2] = min;
		pRasterBand->SetStatIngoreValues(&ignoreValues, 3);
	}
	if (maxstr == "inf" || maxstr == "-inf")
	{
		double* ignoreValues = new double[3];
		ignoreValues[0] = -999;
		ignoreValues[1] = 1.1754943508222875e-038;
		ignoreValues[2] = max;
		pRasterBand->SetStatIngoreValues(&ignoreValues, 3);
	}
	
	pRasterBand->ComputeStatistics(0, &min, &max, &mean, &dev, NULL, NULL);


	prop.pixelMin = min;
	prop.pixelMax = max;
	prop.pixelMean = mean;
	prop.pixelDev = dev;


	GDALDataset *dstDataset = poDriver->CreateCopy(dstFile.c_str(), pDataset, FALSE, NULL, NULL, NULL);

	if (dstDataset->GetProjectionRef() == NULL || strlen(dstDataset->GetProjectionRef()) <= 0)
	{
		OGRSpatialReference sr;
		sr.SetWellKnownGeogCS("WGS84");
		char* srWgs84Wtk = NULL;
		sr.exportToWkt(&srWgs84Wtk);
		dstDataset->SetProjection(srWgs84Wtk);
	}
	GDALClose(dstDataset);

	//解压后删除
	int iLen = strlen(fileNameTif.c_str());
	TCHAR *chRtnTif = new TCHAR[iLen+1];
	mbstowcs(chRtnTif, fileNameTif.c_str(), iLen+1);

	iLen = strlen(dstFile.c_str());
	TCHAR *chRtnPath = new TCHAR[iLen+1];
	mbstowcs(chRtnPath, dstFile.c_str(), iLen+1);

	iLen = strlen(dstZIPFile.c_str());
	TCHAR *chRtnZipPath = new TCHAR[iLen+1];
	mbstowcs(chRtnZipPath, dstZIPFile.c_str(), iLen+1);

	HZIP hz = CreateZip(chRtnZipPath,0);
	ZipAdd(hz,chRtnTif, chRtnPath);
	CloseZip(hz);

	//string wL = "python gdal2tiles.py -p geodetic -r near " + dstFile + " " + dstPath;
	string wL = "gdal2tiles -p geodetic -r near " + dstFile + " " + dstPath;
	QProcess p(0);
	p.start("cmd", QStringList() << "/c" << wL.c_str());
	p.waitForStarted();
	p.waitForFinished();
	string strTemp = QString::fromLocal8Bit(p.readAllStandardOutput()).toStdString();

	QFile::remove(dstFile.c_str());

	QDir dir(dstPath.c_str());
	dir.setFilter(QDir::Dirs);
	int maxLevel=0;
	foreach(QFileInfo fullDir, dir.entryInfoList())
	{
		if(fullDir.fileName() != "." && fullDir.fileName() != ".."&&fullDir.fileName().length()<3)
		{
			int test=fullDir.fileName().toInt();
			if (test>maxLevel)
				maxLevel=test;
		}
	}
	prop.maxDataLevel = maxLevel;
	//int maxLevel = 0;
	//struct _finddata_t fileinfo;    //_finddata_t是一个结构体，要用到#include <io.h>头文件；  
	//long ld;
	//if ((ld = _findfirst((dstPath + "\\*").c_str(), &fileinfo)) != -1l){
	//	do{
	//		if ((fileinfo.attrib&_A_SUBDIR))
	//		{  //如果是文件夹
	//			if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
	//			{
	//				string xx = fileinfo.name;
	//				maxLevel++;
	//			}
	//		}

	//	} while (_findnext(ld, &fileinfo) == 0);
	//	_findclose(ld);
	//}
	//prop.maxDataLevel = maxLevel - 1;

	if (AddRecord(prop, IsExist))
	{
		
		return true;
	} 
	else
	{
		qDebug(QString::fromLocal8Bit("Add record false.").toStdString().c_str());
		return false;
	}

	
}

bool DataImport::AddRecord(OERSProductProperties prop, bool IsExist)
{

	if (!IsExist)
	{
		string dataset=prop.Facility+"_"+prop.ProdSat+"_"+prop.SensorType+"_"+prop.ProductRegion;
		string sql_insert = "insert into raster." + DataImport::dbTable.toStdString() + " (name,facility,resolution,arithmetic,unit,level,time_type,starttime,endtime,hourtime,sta ,sensor,imagehdfspath,rnum,cnum,pixel_max,pixel_min,pixel_mean,pixel_dev,coordinate_system,maxdatalevel,region,lat_max,lat_min,lon_max,lon_min,producttype,productmonth,dataset,resolutionnum) values";
		sql_insert += "( '" + prop.LayerName + "',";
		sql_insert += "'" + prop.Facility + "',";
		sql_insert += "'" + prop.Resolution + "',";
		sql_insert += "'" + prop.ArithmeticType + "',";
		sql_insert += "'" + prop.Unit + "',";
		sql_insert += "'" + prop.ProdLevel + "',";
		sql_insert += QString::number(prop.ProdTimeType).toStdString() + ",";
		sql_insert += "'" + prop.StartTime.toString("yyyy-MM-dd hh:MM:ss").toStdString() + "',";
		sql_insert += "'" + prop.EndTime.toString("yyyy-MM-dd hh:MM:ss").toStdString() + "',";
		sql_insert += QString::number(prop.ProdHour).toStdString() + ",";
		sql_insert += "'" + prop.ProdSat + "',";
		sql_insert += "'" + prop.SensorType + "',";
		sql_insert += "'" + prop.Imagehdfspath + "',";
		sql_insert += QString::number(prop.rnum).toStdString() + ",";
		sql_insert += QString::number(prop.cnum).toStdString() + ",";
		sql_insert += QString::number(prop.pixelMax).toStdString() + ",";
		sql_insert += QString::number(prop.pixelMin).toStdString() + ",";
		sql_insert += QString::number(prop.pixelMean).toStdString() + ",";
		sql_insert += QString::number(prop.pixelDev).toStdString() + ",";
		sql_insert += "'WGS84',";
		sql_insert += QString::number(prop.maxDataLevel).toStdString() + ",";
		sql_insert += "'" + prop.ProductRegion + "',";
		sql_insert += QString::number(prop.ProdLatMax).toStdString() + ",";
		sql_insert += QString::number(prop.ProdLatMin).toStdString() + ",";
		sql_insert += QString::number(prop.ProdLonMax).toStdString() + ",";
		sql_insert += QString::number(prop.ProdLonMin).toStdString() + ",";
		sql_insert += "'" + prop.ProdType + "',";
		sql_insert += QString::number(prop.ProdMonth).toStdString() + ",";
		sql_insert += "'" + dataset + "',";
		sql_insert += QString::number((prop.ProdLatMax-prop.ProdLatMin)/prop.rnum).toStdString() + ")";

		QSqlDatabase dbRaster = QSqlDatabase::database();
		dbRaster = QSqlDatabase::addDatabase("QPSQL");
		dbRaster.setDatabaseName(DataImportUtils::getSettingByNodePath("/paramSetting/database"));
		dbRaster.setHostName(DataImportUtils::getSettingByNodePath("/paramSetting/serverIP"));
		dbRaster.setPort(7359);
		dbRaster.setUserName("postgres");
		dbRaster.setPassword("1q!@hyes0913");
		if (!dbRaster.open())
		{
			qDebug(QString::fromLocal8Bit("raster db open failed.").toStdString().c_str());
			return false;
		}

		/*if (!DataImportUtils::db.isOpen())
		{
			DataImportUtils::db.open();
		}*/
		QSqlQuery query(dbRaster);//(DataImportUtils::db);
		bool isSuccess = query.exec(sql_insert.c_str());
		dbRaster.close();
		return isSuccess;
	}
	else
	{
		string str_Update = "update raster." + DataImport::dbTable.toStdString() + " SET ";
		str_Update += "name= '" + prop.LayerName + "',";
		str_Update += "facility='" + prop.Facility + "',";
		str_Update += "resolution='" + prop.Resolution + "',";
		str_Update += "arithmetic='" + prop.ArithmeticType + "',";
		str_Update += "unit='" + prop.Unit + "',";
		str_Update += "level='" + prop.ProdLevel + "',";
		str_Update += "time_type=" + QString::number(prop.ProdTimeType).toStdString() + ",";
		str_Update += "starttime='" + prop.StartTime.toString("yyyy-MM-dd hh:MM:ss").toStdString() + "',";
		str_Update += "endtime='" + prop.EndTime.toString("yyyy-MM-dd hh:MM:ss").toStdString() + "',";
		str_Update += "hourtime=" + QString::number(prop.ProdHour).toStdString() + ",";
		str_Update += "sta='" + prop.ProdSat + "',";
		str_Update += "sensor='" + prop.SensorType + "',";
		str_Update += "imagehdfspath='" + prop.Imagehdfspath + "',";
		str_Update += "rnum=" + QString::number(prop.rnum).toStdString() + ",";
		str_Update += "cnum=" + QString::number(prop.cnum).toStdString() + ",";
		str_Update += "pixel_max=" + QString::number(prop.pixelMax).toStdString() + ",";
		str_Update += "pixel_min=" + QString::number(prop.pixelMin).toStdString() + ",";
		str_Update += "pixel_mean=" + QString::number(prop.pixelMean).toStdString() + ",";
		str_Update += "pixel_dev=" + QString::number(prop.pixelDev).toStdString() + ",";
		str_Update += "coordinate_system='WGS84',";
		str_Update += "maxdatalevel=" + QString::number(prop.maxDataLevel).toStdString() + ",";
		str_Update += "region='" + prop.ProductRegion + "',";
		str_Update += "lat_max=" + QString::number(prop.ProdLatMax).toStdString() + ",";
		str_Update += "lat_min=" + QString::number(prop.ProdLatMin).toStdString() + ",";
		str_Update += "lon_max=" + QString::number(prop.ProdLonMax).toStdString() + ",";
		str_Update += "lon_min=" + QString::number(prop.ProdLonMin).toStdString() + ",";
		str_Update += "producttype='" + prop.ProdType + "',";
		str_Update += "productmonth=" + QString::number(prop.ProdMonth).toStdString() + " WHERE name='" + prop.LayerName+"'";

		if (!DataImportUtils::db.isOpen())
		{
			DataImportUtils::db.open();
		}
		QSqlQuery query(DataImportUtils::db);
		return query.exec(str_Update.c_str());
	}
}
