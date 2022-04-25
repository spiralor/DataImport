#ifndef OERSPRODUCTLAYER_H
#define OERSPRODUCTLAYER_H

#include <string>
#include <QDate>
//#include "gdalTH_priv.h"
#include "gdal_priv.h"
using namespace std;

struct OERSProductProperties
{
public:
	string LayerName;//
	string ProdLevel;//
	string ProdType;//
	string ProdSat;//
	string SensorType;//
	string Facility;//
	string ArithmeticType;//
	int ProdTimeType;//
	int ProdHour;//
	string Resolution;//
	string ProductRegion;//
	QDateTime StartTime;//
	QDateTime EndTime;//
	double ProdLatMax;
	double ProdLatMin;
	double ProdLonMax;
	double ProdLonMin;
	double LongitudeStep;
	double LatitudeStep;
	string Unit;
	string Imagehdfspath;
	bool userDefinedVPB;
	double pixelMax;
	double pixelMin;
	double pixelMean;
	double pixelDev;
	int maxDataLevel;
	int rnum;
	int cnum;
	int ProdMonth;//
	bool operator==(const OERSProductProperties& properties) const
	{

		return LayerName == properties.LayerName&&ProdLevel == properties.ProdLevel&&ProdType == properties.ProdType
			&&ProdSat == properties.ProdSat&&SensorType == properties.SensorType&&Facility == properties.Facility
			&&ArithmeticType == properties.ArithmeticType&&ProdTimeType == properties.ProdTimeType
			&&ProdHour == properties.ProdHour&&ProductRegion == properties.ProductRegion&&StartTime == properties.StartTime
			&&ProdLonMin == properties.ProdLonMin&&ProdLatMax == properties.ProdLatMax
			&&ProdLatMin == properties.ProdLatMin&&ProdLonMax == properties.ProdLonMax
			&&Resolution == properties.Resolution&&EndTime == properties.EndTime&&Unit == properties.Unit
			&&Imagehdfspath == properties.Imagehdfspath&&userDefinedVPB == properties.userDefinedVPB
			&&pixelMax == properties.pixelMax&&pixelMin == properties.pixelMin&&pixelMean == properties.pixelMean
			&&rnum == properties.rnum&&cnum == properties.cnum&&LongitudeStep==properties.LongitudeStep&&LatitudeStep==properties.LatitudeStep
			&&pixelDev == properties.pixelDev&&maxDataLevel == properties.maxDataLevel;
	}
};

class OERSProductLayer
{
private:
	GDALDataset *pDataset;
	OERSProductProperties m_Properties;

public:

	OERSProductProperties GetProperties()
	{
		return m_Properties;
	}

	void SetProperties(OERSProductProperties pProperties)
	{
		m_Properties = pProperties;
	}

	GDALDataset* GetDataset()
	{
		return pDataset;
	}

	void setLayer(GDALDataset* datasetIn)
	{
		pDataset = datasetIn;
	}

	OERSProductLayer(GDALDataset* pdt, OERSProductProperties pProperties)
	{
		if (pdt != NULL)
		{
			pDataset = pdt;
		}
		m_Properties = pProperties;
	}
};


#endif