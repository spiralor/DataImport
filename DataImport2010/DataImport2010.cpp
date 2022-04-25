// DataImport2010.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <QDir>
#include <QDate>
#include <QtXml>

#include "DataImport.h"
#include "DataImportUtils.h"

#include <QtWidgets/QApplication>

#include <iostream>
#include <string.h>
#include <fstream>
using namespace std;

//�������
void MessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	try
	{
		QString txtMessage;
		QMutex mutex;
		//����
		mutex.lock();
		//����log�����ʽ
		txtMessage += QString("[%1][%2]")
			.arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
			.arg(context.function);

		switch (type) {
		case QtDebugMsg:
			txtMessage += QString("[Debug] %1").arg(msg);
			break;
		case QtWarningMsg:
			txtMessage += QString("[Warning] %1").arg(msg);
			break;
		case QtCriticalMsg:
			txtMessage += QString("[Critical] %1").arg(msg);
			break;
		case QtFatalMsg:
			txtMessage += QString("[Fatal] %1").arg(msg);
			//abort();  //test_HS
			break;
		default:
			txtMessage += QString("[UnKnown] %1").arg(msg);
			break;
		}

		txtMessage += QString("\r\n");
		//������ļ�(д��׷��ģʽ)
		QFile file("Log.txt");
		if (file.open(QIODevice::WriteOnly | QIODevice::Append))
		{
			QTextStream out(&file);
			out << txtMessage;
		}
		file.flush();
		file.close();
		// ����
		mutex.unlock();
	}
	catch (exception e1)
	{
		std::cout << "run error" << endl;
	}

}


int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	qInstallMessageHandler(MessageOutput);

	DataImportUtils::db = QSqlDatabase::addDatabase("QPSQL");
	DataImportUtils::db.setDatabaseName(DataImportUtils::getSettingByNodePath("/paramSetting/database"));
	DataImportUtils::db.setHostName(DataImportUtils::getSettingByNodePath("/paramSetting/serverIP"));
	DataImportUtils::db.setPort(7359);
	DataImportUtils::db.setUserName("postgres");
	DataImportUtils::db.setPassword("1q!@hyes0913");
	if (!DataImportUtils::db.open())
	{
		qDebug(QString::fromLocal8Bit("open failed.").toStdString().c_str());
		return 0;
	}

	DataImport::dbTable = DataImportUtils::getSettingByNodePath("/paramSetting/table");
	string table_check = DataImport::dbTable.toStdString();
	DataImport* dataImport = new DataImport();
	dataImport->Import();

	//QFile file("SourceTiff_file2_chnunique.txt");  
	//if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  
	//	return 0;  

	//ofstream outPath("SourceTiff_file2_path.txt");

	//ofstream outMissing("SourceTiff_file2_missing.txt");

	//QSqlQuery strQuery(DataImportUtils::db);

	//int count=0;
	//QTextStream in(&file);  
	//QString line = in.readLine();  
	//while (!line.isNull()) {  
	//	string te=line.toStdString();
	//	if (te!=""&&!line.contains("_TP_")&&!line.contains("_PRESS_"))
	//	{
	//		string sqlStr="select filepath  from public.filetest where filename='"+te+".HDF'";
	//		bool isSuccess=strQuery.exec(sqlStr.c_str());
	//		strQuery.next();
	//		string filePath=strQuery.value(0).toString().toLocal8Bit().toStdString();
	//		if (filePath!="")
	//		{
	//			outPath << filePath << endl; 
	//		}
	//		else
	//		{
	//			outMissing << te << endl; 
	//		}
	//		count++;
	//		cout<<filePath<<endl;
	//	}
	//	line = in.readLine();  
	//} 
	//file.close();
	//outPath.close();
	//outMissing.close();
	//outPath.clear();
	//outMissing.clear();
	//return 0;
	//QSqlQuery strQuery(DataImportUtils::db);
	//strQuery.exec("select imagehdfspath,name from raster.chn_layer where sensor='VIIRS'");

	//QString pathValue,staValue,sensorValue,nameValue;

	//while(strQuery.next())
	//{
	//	bool success=false;
	//	pathValue = strQuery.value(0).toString();
	//	nameValue = strQuery.value(1).toString();

	//	QString filePath,filePath2,newFileName,folderName,folderName2,oldFileName,oldFileName2;
	//	newFileName=nameValue;

	//	filePath=pathValue.replace("OSGOCEAN","Z:\SourceTiff\TIFF");
	//	filePath=filePath.replace(".tif",".zip");
	//	filePath=filePath.replace("NASA_SNPP_VIIRS","NASA_VIRRS_VIRRS");
	//	filePath2=filePath.replace("NASA_VIRRS_VIRRS","NASA_MERGE_VIRRS");
	//	oldFileName=newFileName.replace("NASA_SNPP_VIIRS","NASA_VIRRS_VIRRS");
	//	oldFileName2=newFileName.replace("NASA_SNPP_VIIRS","NASA_MERGE_VIRRS");
	//	folderName=filePath.replace(".zip","");
	//	folderName2=filePath2.replace(".zip","");

	//	string test=filePath.toStdString();
	//	string test1=filePath2.toStdString();
	//	string test2=newFileName.toStdString();

	//	QFile file(filePath);
	//	if (file.exists())
	//	{
	//		bool ok = file.rename(newFileName);
	//		if(ok)
	//		{
	//			QDir dirFolder(folderName);
	//			bool ok = dirFolder.rename(oldFileName,newFileName);
	//			if (ok)
	//			{
	//				success=true;
	//				cout<<newFileName.toStdString().c_str()<<" success"<<endl;
	//			}
	//		}
	//	}

	//	QFile file2(filePath2);
	//	if (file2.exists())
	//	{
	//		bool ok = file2.rename(newFileName);
	//		if(ok)
	//		{
	//			QDir dirFolder2(folderName2);
	//			bool ok = dirFolder2.rename(oldFileName2,newFileName);
	//			if (ok)
	//			{
	//				success=true;
	//				cout<<newFileName.toStdString().c_str()<<" success"<<endl;
	//			}
	//		}
	//	}
	//	if(!success)
	//	{
	//		qDebug(newFileName.toStdString().c_str());
	//	}
	//}
	getchar();
	return 0;
	
}

