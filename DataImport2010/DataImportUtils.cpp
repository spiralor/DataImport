#include <QtXml>
#include "DataimportUtils.h"
#include <iostream>
using namespace std;

QSqlDatabase DataImportUtils::db = QSqlDatabase::database();

QString DataImportUtils::getSettingByNodePath(string nodePath)
{
	QStringList nodePathList = QString::fromLocal8Bit(nodePath.c_str()).split("/");
	QDomDocument doc("mydocument");
	QFile file("paramSetting.xml");
	if (!doc.setContent(&file))
	{
		file.close();
		return NULL;
	}
	file.close();

	//¸ù½Úµã
	QDomElement docElem = doc.documentElement();
	int Num = nodePathList.count(); //test_HS
	for (int index = 1; index < nodePathList.count(); index++)
	{
		while (!docElem.isNull())
		{
			string test = docElem.tagName().toStdString();//test_HS
			if (docElem.tagName() == nodePathList[index])
			{
				if (index == nodePathList.count() - 1)
				{
					return docElem.text();
				}
				else
				{
					docElem = docElem.firstChild().toElement();
					break;
				}

			}
			else
			{
				docElem = docElem.nextSibling().toElement();
			}

		}
	}
	return NULL;
}