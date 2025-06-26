#pragma once

#include <QDialog>
#include <QMouseEvent>
#include "ui_SettingsDlg.h"
#include "PublicFunc.h"

class SettingsDlg : public QDialog
{
	Q_OBJECT

public:
	SettingsDlg(const ST_ViewParam& stParam, QWidget *parent = nullptr);
	~SettingsDlg();

	ST_ViewParam GetParams() const { return m_stParam; }

protected:
	void initUi();
	void updateUi(const ST_ViewParam& stParam);
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void updateParams();

protected slots:
	void on_btnOK_clicked();
	void on_btnCancel_clicked();

private:
	Ui::SettingsDlgClass ui;

	QPoint m_dragPosition;
	ST_ViewParam m_stParam;
};

