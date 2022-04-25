#ifndef DATAIMPORTUTILS_H
#define DATAIMPORTUTILS_H

#include <QString>
#include <string>
#include <QtSql>
#include <QSqlQuery>

using namespace std;

static class DataImportUtils
{
public:
	DataImportUtils();
	static QString getSettingByNodePath(string nodePath);
	static QSqlDatabase db;

private:

};

#endif