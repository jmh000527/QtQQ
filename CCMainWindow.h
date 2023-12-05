﻿#pragma once

#include <QtWidgets/QWidget>

#include "basicwindow.h"
#include "ui_CCMainWindow.h"

class CCMainWindow : public BasicWindow {
	Q_OBJECT

public:
	CCMainWindow(QString employeeID, QWidget* parent = nullptr);
	~CCMainWindow() override;

private:
	Ui::CCMainWindowClass ui{};

	void initTimer();
	void initColtrol();
	void initContactTree();
	void setUserName(const QString& username) const;
	void setUserLevelPixmap(int level) const;
	void setUserHeadPixmap(const QString& headPath) const;
	void setUserStatusMenuIcon(const QString& statusPath) const;

	QString getHeadPicturePath();
	QString getUsername();

	QWidget* createOtherAppExtension(const QString& appPath, const QString& appName);

	void resizeEvent(QResizeEvent* event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

	void updateSearchStyle() const;
	void addDepartment(QTreeWidgetItem* pRootGroupItem, const int DepID);

	QString m_employeeID;	//账号 或QQ 号

private slots:
	void onAppIconClicked() const;

	void onItemClicked(QTreeWidgetItem* item, int column);
	void onItemExpanded(QTreeWidgetItem* item);
	void onItemCollapsed(QTreeWidgetItem* item);
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
};
