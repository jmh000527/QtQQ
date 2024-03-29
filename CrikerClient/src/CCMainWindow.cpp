﻿#include "CCMainWindow.h"

#include <QDateTime>
#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QMap>
#include <QSqlQuery>

#include "ChatWindowShell.h"
#include "CommonUtils.h"
#include "ContactItem.h"
#include "NotifyManager.h"
#include "RootContactItem.h"
#include "SkinWindow.h"
#include "SysTray.h"
#include "UserManager.h"
#include "WindowManager.h"
#include "WinToastHandlerCallBack.h"

#include "WinToast/wintoastlib.h"

CCMainWindow::CCMainWindow(QWidget* parent)
	: BasicWindow{ parent } {
	ui.setupUi(this);
	//存储MainWindow指针
	WindowManager::getInstance()->setMainWindowPointer(this);

	setWindowFlags(windowFlags() | Qt::Tool);
	loadStyleSheet("CCMainWindow");

	setUserHeadPixmap();

	initColtrol();
	initTimer();

	//初始化搜索结果展示部件
	// strs << "A1" << "A2" << "A3" << "A4" << "B1" << "B2" << "B3" << "B4";
	// strs << "C1" << "C2" << "C3" << "C4" << "D1" << "D2" << "D3" << "D4";
	auto userList = UserManager::getCurrentUserFriendList();
	for(const auto& user : userList) {
		strs << user.getName().c_str();
	}
	m_pSearchResult = new SearchResult{ this };

	//TabBar相关
	ui.stackedWidget->setCurrentIndex(0);
}

CCMainWindow::~CCMainWindow() {
	// // 在应用程序退出时清理 WinToast
	// WinToastLib::WinToast::instance()->finalize();
}

void CCMainWindow::setLineEditText(QString text) const {
	ui.searchLineEdit->setText(text);
}

void CCMainWindow::loadStyleSheet(const QString& sheetName) {
	m_styleSheetName = sheetName;
	QFile file{ ":/Resources/QSS/" + sheetName + ".css" };
	file.open(QFile::ReadOnly);

	// QPixmap messageSelected{ ":/Resources/MainWindow/tab/icon_last_selected.png" };
	// QPixmap messageNormal{ ":/Resources/MainWindow/tab/icon_last_normal.png" };
	// QPixmap contactSelected{ ":/Resources/MainWindow/tab/icon_group_selected.png" };
	// QPixmap contactNormal{ ":/Resources/MainWindow/tab/icon_group_normal.png" };
	//
	// messageSelected = CommonUtils::replaceColorWithBackground(messageSelected);
	// contactSelected = CommonUtils::replaceColorWithBackground(contactSelected);
	//
	// QObject::connect(ui.btnMessage, &QPushButton::toggled, [this, messageNormal, messageSelected](bool checked) {
	// 	if (checked) {
	// 		// 按钮被选中时的逻辑
	// 		ui.btnMessage->setIcon(messageSelected);
	// 	}
	// 	else {
	// 		// 按钮取消选中时的逻辑
	// 		ui.btnMessage->setIcon(messageNormal);
	// 	}
	// 				 });
	//
	// QObject::connect(ui.btnContact, &QPushButton::toggled, [this, contactNormal, contactSelected](bool checked) {
	// 	if (checked) {
	// 		// 按钮被选中时的逻辑
	// 		ui.btnContact->setIcon(contactSelected);
	// 	}
	// 	else {
	// 		// 按钮取消选中时的逻辑
	// 		ui.btnContact->setIcon(contactNormal);
	// 	}
	// 				 });

	if (file.isOpen()) {
		setStyleSheet("");
		QString qStyleSheet{ QLatin1String{ file.readAll() } };

		//获取用户当前皮肤的RGB
		const QString r{ QString::number(m_colorBackground.red()) };
		const QString g{ QString::number(m_colorBackground.green()) };
		const QString b{ QString::number(m_colorBackground.blue()) };

		const auto increaseValue{ 230 };
		qStyleSheet += QString("QWidget[titleSkin = true] {\
								background-color: rgb(%1, %2, %3);\
								border-top-left-radius: 4px;\
								border-top-right-radius: 4px;\
								border-bottom: none;\
								}\
								QWidget[bottomSkin = true] {\
								background-color: rgb(%4, %5, %6);\
								border-bottom-left-radius: 4px;\
								border-bottom-right-radius: 4px;\
								border: 1px solid rgb(%1, %2, %3);\
								border-top: none;\
								}\
								QWidget#CCMainWindowClass {\
								background-color: rgb(%1, %2, %3);\
								border-radius: 4px;\
								}\
								QWidget#barWidget{\
								background-color: rgb(%4, %5, %6);\
								}\
								QPushButton#btnMessage, #btnContact{\
								background-color: rgb(%4, %5, %6);\
								border-none;\
						        background: transparent;\
								}\
							    QPushButton#btnMessage, #btnContact:hover{\
								background-color: rgb(205, 205, 205);\
								border-none;\
						        background: transparent;\
							    }\
							    QPushButton:pressed, QPushButton:checked{\
								background-color: rgb(%1, %2, %3);\
							    }\
								QTreeView {\
								border-style: none;\
								}\
								QTreeView::item{\
								color: rgba(255, 255, 255, 0);\
								}\
								QTreeView::item:selected:active{\
								background-color: rgb(%1, %2, %3);\
								}\
								QTreeView::item:selected:active,\
								QTreeView::item:hover{\
								background-color: rgb(%4, %5, %6);\
								}\
								QListWidget#msgListWidget {\
								border-style: none;\
								outline: none;\
								}\
								QListWidget#msgListWidget::item{\
								color: rgba(255, 255, 255, 0);\
								}\
								QListWidget#msgListWidget::item:selected:active{\
								background-color: rgb(%1, %2, %3);\
								}\
								QListWidget#msgListWidget::item:selected:active,\
								QListWidget#msgListWidget::item:hover{\
								background-color: rgb(%4, %5, %6);\
								}"
			).arg(r).arg(g).arg(b)
			 .arg(qMin(r.toInt() / 10 + increaseValue, 255))
			 .arg(qMin(g.toInt() / 10 + increaseValue, 255))
			 .arg(qMin(b.toInt() / 10 + increaseValue, 255));

		setStyleSheet(qStyleSheet);
	}

	file.close();
}

void CCMainWindow::initTimer() {
	auto* timer{ new QTimer{ this } };
	timer->setInterval(std::chrono::seconds(1));
	connect(timer, &QTimer::timeout,
	        [this]() {
		        static int level{};
		        if (level == 99) {
			        level = 0;
		        }
		        level++;

		        // setUserLevelPixmap(level);
	        }
	);

	timer->start();
}

void CCMainWindow::initColtrol() {
	ui.treeWidget->setStyle(new CustomProxyStyle{ this });

	// setUserLevelPixmap(0);
	setUserStatusMenuIcon(":/Resources/MainWindow/StatusSucceeded.png");

	const auto appUpLayout{ new QHBoxLayout{ this } };

	appUpLayout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
	appUpLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_7.png", "app_7"));
	// appUpLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_2.png", "app_2"));
	// appUpLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_3.png", "app_3"));
	// appUpLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_4.png", "app_4"));
	// appUpLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_5.png", "app_5"));
	// appUpLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_6.png", "app_6"));
	appUpLayout->addStretch();
	appUpLayout->setSpacing(2);
	appUpLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/skin.png", "app_skin"));

	ui.appWidget->setLayout(appUpLayout);

	const auto appBottomLayout{ new QHBoxLayout{ this } };

	appBottomLayout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
	// appBottomLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_10.png", "app_10"));
	appBottomLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_8.png", "app_8"));
	// appBottomLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_11.png", "app_11"));
	// appBottomLayout->addWidget(createOtherAppExtension(":/Resources/MainWindow/app/app_9.png", "app_9"));

	ui.bottomLayout_up->addLayout(appBottomLayout);
	ui.bottomLayout_up->addStretch();

	//个性签名
	ui.lineEdit->installEventFilter(this);
	//搜索
	ui.searchLineEdit->installEventFilter(this);

	//单独更新搜索部件的颜色
	updateSearchStyle();

	connect(ui.sysmin, SIGNAL(clicked(bool)), this, SLOT(onShowHide(bool)));
	connect(ui.sysclose, SIGNAL(clicked(bool)), this, SLOT(onShowQuit(bool)));

	//实现在更换皮肤时，同时更新搜索部件底色
	connect(NotifyManager::getInstance(), &NotifyManager::signalSkinChanged, this, &CCMainWindow::updateSearchStyle);

	//系统托盘
	m_sysTray =  new SysTray{ this };

	//初始化联系人部件
	initContactTree();

	//初始化barWidget信号
	connect(ui.btnMessage, &QPushButton::clicked, [this]() {
		ui.stackedWidget->setCurrentIndex(0);
	});
	connect(ui.btnContact, &QPushButton::clicked, [this]() {
		ui.stackedWidget->setCurrentIndex(1);
		if(ui.btnMessage->isChecked()) {
			ui.btnMessage->setChecked(false);
		}
	});
}

void CCMainWindow::initContactTree() {
	//展开与收纳的信号
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,
	        SLOT(onItemClicked(QTreeWidgetItem*, int)));
	connect(ui.treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this,
	        SLOT(onItemExpanded(QTreeWidgetItem*)));
	connect(ui.treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this,
	        SLOT(onItemCollapsed(QTreeWidgetItem*)));
	connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this,
	        SLOT(onItemDoubleClicked(QTreeWidgetItem *, int)));

	//根节点
	const auto pRootFriendItem{ new QTreeWidgetItem };
	pRootFriendItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	pRootFriendItem->setData(0, Qt::UserRole, QVariant{ 0 });

	const auto pRootGroupItem{ new QTreeWidgetItem };
	pRootGroupItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	pRootGroupItem->setData(0, Qt::UserRole, QVariant{ 0 });

	//向treeWidget添加当前账户好友
	for (const auto& user : UserManager::getCurrentUserFriendList()) {
		addFriends(pRootFriendItem, user);
	}

	//向treeWidget添加当前账户群组
	for (const auto& group : UserManager::getCurrentUserGroupList()) {
		addGroups(pRootGroupItem, group);
	}

	//插入分组节点
	const auto pItemFriend{ new RootContactItem{ true, ui.treeWidget } };
	pItemFriend->setText(QString{ "好友" });
	const auto pItemGroup{ new RootContactItem{ true, ui.treeWidget } };
	pItemGroup->setText(QString{ "群组" });

	ui.treeWidget->addTopLevelItem(pRootFriendItem);
	ui.treeWidget->addTopLevelItem(pRootGroupItem);

	ui.treeWidget->setItemWidget(pRootFriendItem, 0, pItemFriend);
	ui.treeWidget->setItemWidget(pRootGroupItem, 0, pItemGroup);
}

void CCMainWindow::initShowResult() {
	const auto showPoint = ui.searchLineEdit->mapTo(this, ui.searchLineEdit->geometry().bottomLeft());
	const auto width = ui.searchWidget->width();

	m_pSearchResult->move(showPoint.x(), showPoint.y());
	m_pSearchResult->setFixedWidth(width);
	onSearchLineEditTextChanged(ui.searchLineEdit->text());
	m_pSearchResult->setFixedHeight(ui.searchLineEdit->height() * 7);

	// 连接 textChanged 信号到槽函数
	connect(ui.searchLineEdit, &QLineEdit::textChanged, this, &CCMainWindow::onSearchLineEditTextChanged);
}

void CCMainWindow::setUserName(const QString& username) const {
	//文本过长则省略过长部分
	const QString elidedUserName{
		ui.nameLabel->fontMetrics().elidedText(username, Qt::ElideRight, ui.nameLabel->width()) };

	ui.nameLabel->setText(elidedUserName);
}

// void CCMainWindow::setUserLevelPixmap(const int level) const {
// 	QPixmap levePixmap{ ui.levelBtn->size() };
// 	levePixmap.fill(Qt::transparent);
//
// 	QPainter painter{ &levePixmap };
// 	painter.drawPixmap(0, 4, QPixmap{ ":/Resources/MainWindow/lv.png" });
//
// 	const auto unitNum{ level % 10 }; //个位数
// 	const auto tenNum{ level / 10 % 10 }; //十位数
//
// 	//十位，截取图片中的部分进行绘制
// 	painter.drawPixmap(10, 4, QPixmap{ ":/Resources/MainWindow/levelvalue.png" }, tenNum * 6, 0, 6, 7);
//
// 	//个位，同上
// 	painter.drawPixmap(16, 4, QPixmap{ ":/Resources/MainWindow/levelvalue.png" }, unitNum * 6, 0, 6, 7);
//
// 	ui.levelBtn->setIcon(levePixmap);
// 	ui.levelBtn->setIconSize(ui.levelBtn->size());
// }

void CCMainWindow::setUserHeadPixmap() const {
	const QPixmap headImage{ CommonUtils::base64ToQPixmap(UserManager::getCurrentUser().getHeadImage()) };

	QPixmap mask;
	mask.load(":/Resources/MainWindow/head_mask.png");

	ui.headLabel->setPixmap(getRoundedImage(headImage, mask, ui.headLabel->size()));
}

void CCMainWindow::setUserStatusMenuIcon(const QString& statusPath) const {
	QPixmap statusBtnPixmap{ ui.statusBtn->size() };
	statusBtnPixmap.fill(Qt::transparent);
	QPainter painter{ &statusBtnPixmap };
	painter.drawPixmap(4, 4, QPixmap{ statusPath });

	ui.statusBtn->setIcon(statusBtnPixmap);
	ui.statusBtn->setIconSize(ui.statusBtn->size());
}

QWidget* CCMainWindow::createOtherAppExtension(const QString& appPath, const QString& appName) {
	const auto button{ new QPushButton{ this } };
	button->setFixedSize(QSize{ 20, 20 });

	QPixmap pixmap{ button->size() };
	pixmap.fill(Qt::transparent);

	QPainter painter{ &pixmap };
	const QPixmap appPixmap{ appPath };

	painter.drawPixmap((button->width() - appPixmap.width()) / 2, (button->height() - appPixmap.height()) / 2,
	                   appPixmap);
	button->setIcon(pixmap);
	button->setIconSize(button->size());
	button->setObjectName(appName);
	button->setProperty("hasBorder", true);

	connect(button, &QPushButton::clicked, this, &CCMainWindow::onAppIconClicked);

	return button;
}

void CCMainWindow::resizeEvent(QResizeEvent* event) {
	setUserName(QString{ UserManager::getCurrentUser().getName().c_str() });
	initShowResult();
	// m_pSearchResult->hide();                      //隐藏展示

	return BasicWindow::resizeEvent(event);
}

void CCMainWindow::showEvent(QShowEvent* event) {
	initShowResult();
	m_pSearchResult->hide(); //隐藏展示

	return BasicWindow::showEvent(event);
}

bool CCMainWindow::eventFilter(QObject* watched, QEvent* event) {
	if (ui.searchLineEdit == watched) {
		//键盘焦点事件
		if (event->type() == QEvent::FocusIn) {
			//获取用户当前皮肤的RGB
			const QString r{ QString::number(m_colorBackground.red()) };
			const QString g{ QString::number(m_colorBackground.green()) };
			const QString b{ QString::number(m_colorBackground.blue()) };

			ui.searchWidget->setStyleSheet(QString{ "QWidget#searchWidget{\
													background-color: rgb(255, 255, 255);\
													border-bottom: 1px rgb(%1, %2, %3);\
													}\
													QWidget#searchLineEdit{\
													border-bottom: 1px rgb(%1, %2, %3);\
													}\
													QPushButton#searchBtn{\
													border-image:url(:/Resources/MainWindow/search/main_search_deldown.png)\
													}\
													QPushButton#searchBtn:hover{\
													border-image:url(:/Resources/MainWindow/search/main_search_delhighlight.png)\
													}\
													QPushButton#searchBtn:pressed{\
													border-image:url(:/Resources/MainWindow/search/main_search_delhighdown.png)\
													}" }
			                               .arg(r)
			                               .arg(g)
			                               .arg(b));
			//搜索结果展示部件
			m_pSearchResult->show(); //如果获得焦点  就展示下拉界面
		} else if (event->type() == QEvent::FocusOut) {
			//还原搜索部件样式
			updateSearchStyle();

			// 搜索结果展示部件
			if (!m_pSearchResult->underMouse()) {
				m_pSearchResult->hide();
			}
		}
	}

	return BasicWindow::eventFilter(watched, event);
}

void CCMainWindow::mousePressEvent(QMouseEvent* event) {
	if (qApp->widgetAt(event->pos()) != ui.searchLineEdit && ui.searchLineEdit->hasFocus()) {
		ui.searchLineEdit->clearFocus();
	} else if (qApp->widgetAt(event->pos()) != ui.lineEdit && ui.lineEdit->hasFocus()) {
		ui.lineEdit->clearFocus();
	}

	return BasicWindow::mousePressEvent(event);
}

void CCMainWindow::updateSearchStyle() const {
	//获取用户当前皮肤的RGB
	const QString r{ QString::number(m_colorBackground.red()) };
	const QString g{ QString::number(m_colorBackground.green()) };
	const QString b{ QString::number(m_colorBackground.blue()) };
	const auto increaseValue{ 230 };

	ui.searchWidget->setStyleSheet(QString{ "QWidget#searchWidget {\
											background-color:rgb(%1, %2, %3);\
											border-bottom:1px rgb(%1, %2, %3);\
											}\
											QPushButton#searchBtn {\
											border-image:url(:Resources/MainWindow/search/search_icon.png);\
											}" }
	                               .arg(qMin(r.toInt() / 10 + increaseValue, 255))
	                               .arg(qMin(g.toInt() / 10 + increaseValue, 255))
	                               .arg(qMin(b.toInt() / 10 + increaseValue, 255)));
}

void CCMainWindow::addDepartment(QTreeWidgetItem* pRootGroupItem, const int depID) const {
	const auto pChildItem{ new QTreeWidgetItem };

	//添加子节点，子项数据为1
	pChildItem->setData(0, Qt::UserRole, 1);
	pChildItem->setData(0, Qt::UserRole + 1, depID);

	//获取部门头像路径
	QSqlQuery queryDepPic(
		QString("SELECT picture,department_name FROM tab_department WHERE departmentID=%1").arg(depID));
	queryDepPic.exec();
	queryDepPic.first();

	QPixmap groupPix;
	groupPix.load(queryDepPic.value(0).toString());

	//获取部门名称
	QSqlQuery queryDepName(QString("SELECT department_name FROM tab_department WHERE departmentID=%1").arg(depID));
	queryDepName.exec();
	queryDepName.first();

	//设置头像
	const auto pContactItem{ new ContactItem{ ui.treeWidget } };
	QPixmap pixmap{ ":/Resources/MainWindow/head_mask.png" };
	pContactItem->setHeadPixmap(getRoundedImage(groupPix, pixmap, pContactItem->getHeadLabelSize()));
	pContactItem->setUsername(queryDepName.value(0).toString());

	pRootGroupItem->addChild(pChildItem);
	ui.treeWidget->setItemWidget(pChildItem, 0, pContactItem);

	// m_groupMap.insert(pChildItem, departmentName);
}

void CCMainWindow::addFriends(QTreeWidgetItem* pRootGroupItem, const User& user) {
	const auto pChildItem{ new QTreeWidgetItem };

	//添加子节点，子项数据为1
	pChildItem->setData(0, Qt::UserRole, 1);
	pChildItem->setData(0, Qt::UserRole + 1, user.getId());

	const auto pContactItem{ new ContactItem{ ui.treeWidget } };
	pRootGroupItem->addChild(pChildItem);
	ui.treeWidget->setItemWidget(pChildItem, 0, pContactItem);

	const QPixmap headImage{ CommonUtils::base64ToQPixmap(user.getHeadImage()) };
	QPixmap mask{ ":/Resources/MainWindow/head_mask.png" };

	pContactItem->setHeadPixmap(getRoundedImage(headImage, mask, pContactItem->getHeadLabelSize()));
	pContactItem->setUsername(QString{ user.getName().c_str() });
}

void CCMainWindow::addGroups(QTreeWidgetItem* pRootGroupItem, const Group& group) {
	const auto pChildItem{ new QTreeWidgetItem };

	//添加子节点，子项数据为1
	pChildItem->setData(0, Qt::UserRole, 1);
	pChildItem->setData(0, Qt::UserRole + 1, group.getId());

	const auto pContactItem{ new ContactItem{ ui.treeWidget } };
	// pContactItem->setHeadPixmap(getRoundedImage(groupPix, pixmap, pContactItem->getHeadLabelSize()));
	pContactItem->setUsername(QString{ group.getName().c_str() });

	pRootGroupItem->addChild(pChildItem);
	ui.treeWidget->setItemWidget(pChildItem, 0, pContactItem);
}

void CCMainWindow::addUserMessage(const User& user, const QString& msg, const QString& time) {
	const auto pContactItem{ new ContactItem{ ui.msgListWidget } };
	const QPixmap headImage{ CommonUtils::base64ToQPixmap(user.getHeadImage()) };
	QPixmap mask{ ":/Resources/MainWindow/head_mask.png" };

	const QDateTime dateTime = QDateTime::fromSecsSinceEpoch(time.toInt());
	const QTime time1 = dateTime.time();
	const QString timeString = time1.toString("hh:mm");

	pContactItem->setHeadPixmap(getRoundedImage(headImage, mask, pContactItem->getHeadLabelSize()));
	pContactItem->setUsername(QString{ user.getName().c_str() });
	pContactItem->setSignature("(" + timeString + ") " + msg);

	// 创建QListWidgetItem，并将自定义的QWidget子类设置为其小部件
	QListWidgetItem* item1 = new QListWidgetItem{};

	QSize sizeHint = pContactItem->sizeHint();
	item1->setSizeHint(QSize{ sizeHint.width(), 54 });
	ui.msgListWidget->insertItem(0, item1);
	ui.msgListWidget->setItemWidget(item1, pContactItem);

	// 更新布局
	ui.msgListWidget->update();

	//系统通知中心推送
	QIcon icon{ headImage };
	m_sysTray->showMessage(user.getName().c_str(), msg, icon);
}

void CCMainWindow::addGroupMessage(const Group& group, const QString& msg, const QString& time) {
	const auto pContactItem{ new ContactItem{ ui.msgListWidget } };
	// const QPixmap headImage{ CommonUtils::base64ToQPixmap(group.getHeadImage()) };
	// QPixmap mask{ ":/Resources/MainWindow/head_mask.png" };

	const QDateTime dateTime = QDateTime::fromSecsSinceEpoch(time.toInt());
	const QTime time1 = dateTime.time();
	const QString timeString = time1.toString("hh:mm");

	// pContactItem->setHeadPixmap(getRoundedImage(headImage, mask, pContactItem->getHeadLabelSize()));
	pContactItem->setUsername(QString{ group.getName().c_str() });
	pContactItem->setSignature("(" + timeString + ") " + msg);

	// 创建QListWidgetItem，并将自定义的QWidget子类设置为其小部件
	QListWidgetItem* item1 = new QListWidgetItem{};

	QSize sizeHint = pContactItem->sizeHint();
	item1->setSizeHint(QSize{ sizeHint.width(), 54 });
	ui.msgListWidget->insertItem(0, item1);
	ui.msgListWidget->setItemWidget(item1, pContactItem);

	// 更新布局
	ui.msgListWidget->update();

	//系统通知中心推送
	// QIcon icon{ headImage };
	m_sysTray->showMessage(group.getName().c_str(), msg, m_sysTray->icon());
}

void CCMainWindow::pushToSystem(const QString msg, const QString time, const QString senderId) {
	// // 初始化 WinToast
	// WinToastLib::WinToast::instance()->initialize();
	//
	// // 创建通知
	// WinToastLib::WinToastTemplate templ = WinToastLib::WinToastTemplate(WinToastLib::WinToastTemplate::ImageAndText04);
	// templ.setImagePath(L"path_to_your_image.png"); // 设置通知图像
	// templ.setTextField(L"title", WinToastLib::WinToastTemplate::FirstLine); // 设置通知标题
	// templ.setTextField(L"message", WinToastLib::WinToastTemplate::SecondLine); // 设置通知消息
	//
	// // 发送通知
	// WinToastLib::WinToast::instance()->showToast(templ, new WinToastLib::IWinToastHandler());

	// NotifyManager::showNotification(QString{ "Criker" }.toStdWString(), QString{ "Criker" }.toStdWString(), msg.toStdWString(), QString{ "D:/Qt/dcm/labels/1_0.jpg" }.toStdWString());
}

void CCMainWindow::onAppIconClicked() const {
	if (sender()->objectName() == "app_skin") {
		const auto skinWindow{ new SkinWindow };
		skinWindow->show();
	}
}

void CCMainWindow::onItemClicked(QTreeWidgetItem* item, int column) {
	//数据为0表示根项，数据为1表示子项
	const auto isChild{ item->data(0, Qt::UserRole).toBool() };
	if (!isChild) {
		item->setExpanded(!item->isExpanded());
	}

	update();
}

void CCMainWindow::onItemExpanded(QTreeWidgetItem* item) {
	const auto isChild{ item->data(0, Qt::UserRole).toBool() };
	if (!isChild) {
		const auto pRootContactItem{ dynamic_cast<RootContactItem*>(ui.treeWidget->itemWidget(item, 0)) };
		if (pRootContactItem != nullptr) {
			pRootContactItem->setExpanded(true);
		}
	}

	update();
}

void CCMainWindow::onItemCollapsed(QTreeWidgetItem* item) {
	const auto isChild{ item->data(0, Qt::UserRole).toBool() };
	if (!isChild) {
		const auto pRootContactItem{ dynamic_cast<RootContactItem*>(ui.treeWidget->itemWidget(item, 0)) };
		if (pRootContactItem != nullptr) {
			pRootContactItem->setExpanded(false);
		}
	}

	update();
}

void CCMainWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
	if (const auto isChild = item->data(0, Qt::UserRole).toBool(); isChild == true && item != nullptr) {
		// 循环向上查找父节点，直到找到顶级节点
		const auto itemClicked = item;
		while (item->parent()) {
			item = item->parent();
		}

		// 获取设置在 TopLevelItem 上的 ItemWidget
		if (QTreeWidgetItem* topLevelItem = item) {
			auto* topLevelItemWidget = dynamic_cast<RootContactItem*>(ui.treeWidget->itemWidget(topLevelItem, 0));
			const auto& topLevelItemText = topLevelItemWidget->text();
			// qDebug() << "Clicked on TopLevelItem: " << topLevelItemText;

			WindowManager::getInstance()->addNewChatWindow(itemClicked->data(0, Qt::UserRole + 1).toString(),
			                                               topLevelItemText == "群组");
			// 在这里可以根据 topLevelItem 进行不同的处理
		}
	}
}

void CCMainWindow::onSearchLineEditTextChanged(const QString& arg1) const {
	QStringList ss;
	QString s;

	//遍历我们存储的所有信息 后续是数据库查询
	for (auto i = 0; i < strs.size(); i++) {
		s = strs.at(i);

		//模糊匹配 当前的信息被包含在这一元素内
		if (s.contains(arg1)) {
			ss.append(s); //存储在新的list内 准备显示
		}
	}

	m_pSearchResult->strs = ss; //下拉界面的变量赋值
	m_pSearchResult->showTable(); //下拉界面展示查询到的信息
}

void CCMainWindow::onAddMessage(const QString msg, const QString time, const QString senderId) {
	if(UserManager::isGroupChat(senderId)) {
		const Group sender{ UserManager::getGroup(senderId) };
		addGroupMessage(sender, msg, time);
	} else {
		const User sender{ UserManager::getFriend(senderId) };
		addUserMessage(sender, msg, time);
	}
}
