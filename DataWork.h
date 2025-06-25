#pragma once

#include <QObject>

class DataWork  : public QObject
{
	Q_OBJECT

public:
	DataWork(QObject *parent);
	~DataWork();

public Q_SLOTS:
	void onAppendData(const QByteArray& data);
	void onDoWork();
};

