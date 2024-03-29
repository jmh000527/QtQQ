#include "ChatWindow.h"

#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QToolTip>
#include <QScrollBar>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "ChatWindowShell.h"
#include "CommonUtils.h"
#include "ContactItem.h"
#include "Group.h"
#include "MsgType.h"
#include "RootContactItem.h"
#include "TcpClient.h"
#include "UserManager.h"
#include "WindowManager.h"

ChatWindow::ChatWindow(QWidget* parent, const QString& uid, bool isGroupChat)
	: QWidget{ parent }, m_chatId{ uid }, m_isGroupChat{ isGroupChat } {
	ui.setupUi(this);
	WindowManager::getInstance()->addWindowName(this, m_chatId);
	setAttribute(Qt::WA_DeleteOnClose);

	// initGroupChatStatus();
	initControl();
}

ChatWindow::~ChatWindow() {
	WindowManager::getInstance()->deleteWindowByName(m_chatId);
}

void ChatWindow::addEmojiImage(QString unicodeEmoji) const {
	ui.textEdit->setFocus();
	// ui.textEdit->addEmojiUrl(emojiNum);

	// Unicode Emoji
	// 插入文字
	// ui.textEdit->insertPlainText("Hello, ");

	// 获取当前光标位置
	QTextCursor cursor = ui.textEdit->textCursor();

	// // 在光标位置插入Unicode Emoji（这里以笑脸Emoji为例）
	// char32_t  t[] = { 0x1F601,0x0 };
	cursor.insertText(unicodeEmoji);

	// 插入更多文字
	// cursor.insertImage(QImage{ ":/Resources/MainWindow/app/app_7.png" });
}

void ChatWindow::setWindowName(const QString& name) {
	ui.nameLabel->setText(name);
}

void ChatWindow::addGroupUser(QTreeWidgetItem* pRootItem, const GroupUser& groupUser) {
	auto* pChild = new QTreeWidgetItem{};

	//添加子节点
	pChild->setData(0, Qt::UserRole, 1);
	pChild->setData(0, Qt::UserRole + 1, groupUser.getId());
	auto* pContactItem = new ContactItem(ui.treeWidget);

	QPixmap mask;
	mask.load(":/Resources/MainWindow/head_mask.png");
	// QImage imageHead;
	// imageHead.load(strPicPath);
	pContactItem->setHeadPixmap(CommonUtils::getRoundedImage(CommonUtils::base64ToQPixmap(groupUser.getHeadImage()),
	                                                         mask, pContactItem->getHeadLabelSize()));
	pContactItem->setUsername(groupUser.getName().c_str());
	// pContactItem->setSignature(strSign);

	pRootItem->addChild(pChild);
	ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

	const QString str = pContactItem->getUsername();
	m_groupPeopleMap.insert(pChild, str);
}

void ChatWindow::initP2PChat() {
	// QPixmap pixSkin;
	// pixSkin.load(":/Resources/MainWindow/skin.png");
	// // ui.widget->setFixedSize(pixSkin.size());
	//
	// auto* skinLabel = new QLabel(ui.widget);
	// skinLabel->setPixmap(pixSkin);
	// skinLabel->setFixedSize(ui.widget->size());

	ui.widget->hide();
}

int ChatWindow::getCompDepID() {
	QSqlQuery queryDepID(
		QString("SELECT departmentID FROM tab_department WHERE department_name='%1'").
		arg(QString::fromLocal8Bit("公司群")));
	queryDepID.exec();
	queryDepID.next();

	return queryDepID.value(0).toInt();
}

void ChatWindow::initChatWindow() {
	auto* pRootItem = new QTreeWidgetItem();
	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	pRootItem->setData(0, Qt::UserRole, 0);
	auto* pItemName = new RootContactItem(false, ui.treeWidget);
	ui.treeWidget->setFixedHeight(646);

	// //当前聊天的群组名或人名
	// QString strGroupName;
	// QSqlQuery queryGroupName(QString("SELECT department_name FROM tab_department WHERE departmentID=%1").arg(m_chatId));
	// queryGroupName.exec();
	// if (queryGroupName.first()) {
	// 	strGroupName = queryGroupName.value(0).toString();
	// }

	// int nEmployeeNum = 0;

	// //获取员工人数
	// QSqlQueryModel queryEmployeeModel;
	// //公司群
	// if (getCompDepID() == m_chatId.toInt()) {
	// 	queryEmployeeModel.setQuery("SELECT employeeID FROM tab_employees WHERE `status`=1");
	// } else {
	// 	queryEmployeeModel.setQuery(
	// 		QString("SELECT employeeID FROM tab_employees WHERE `status`=1 AND departmentID=%1").arg(m_chatId));
	// }
	// nEmployeeNum = queryEmployeeModel.rowCount();
	// QString qsGroupName = QString{ "%1 %2/%3" }.arg(strGroupName).arg(0).arg(nEmployeeNum);

	Group currentGroup = UserManager::getGroup(m_chatId);
	// const auto& userGroupList = UserManager::getCurrentUserGroupList();
	// const auto iter = std::ranges::find_if(userGroupList,
	//                                        [this](const Group& group) { return group.getId() == m_chatId.toInt(); });
	// if (iter != userGroupList.end()) {
	// 	currentGroup = *iter;
	// }

	pItemName->setText(currentGroup.getName().c_str());

	//插入分组节点
	ui.treeWidget->addTopLevelItem(pRootItem);
	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);

	//展开
	pRootItem->setExpanded(true);

	const auto& currentGroupUsers{ currentGroup.getUsers() };
	for (size_t i{}; i < currentGroupUsers.size(); ++i) {
		addGroupUser(pRootItem, currentGroupUsers.at(i));
	}
}

void ChatWindow::initControl() {
	loadStyleSheet("TalkWindow");
	QList<int> rightWidgetSize;
	rightWidgetSize << 600 << 138;
	ui.bodySplitter->setSizes(rightWidgetSize);
	ui.textEdit->setFontPointSize(10);
	ui.textEdit->setFocus();

	// 禁用垂直滚动条
	ui.listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.listWidget->verticalScrollBar()->setStyleSheet("QScrollBar{width:10px;}");
	// ui.treeWidget->verticalScrollBar()->setStyleSheet("QScrollBar{width:10px;}");
	ui.scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar{width:10px;}");

	// // 设置 resize mode 为 Fixed
	// ui.listWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
	// ui.listWidget->setResizeMode(QListView::Fixed);

	connect(ui.sysmin, SIGNAL(clicked(bool)), parent(), SLOT(onShowMin(bool)));
	connect(ui.sysclose, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));
	connect(ui.closeBtn, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));
	connect(ui.faceBtn, SIGNAL(clicked(bool)), parent(), SLOT(onEmojiBtnClicked(bool)));
	connect(ui.sendBtn, SIGNAL(clicked(bool)), this, SLOT(onSendBtnClicked(bool)));
	connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this,
	        SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));

	if (m_isGroupChat) {
		initChatWindow();
	} else {
		initP2PChat();
	}
}

void ChatWindow::loadStyleSheet(const QString& sheetName) {
	QFile file{ ":/Resources/QSS/" + sheetName + ".css" };
	file.open(QFile::ReadOnly);

	if (file.isOpen()) {
		setStyleSheet("");
		QString qStyleSheet{ QLatin1String{ file.readAll() } };

		//获取用户当前皮肤的RGB
		const QColor backgroundColor{ CommonUtils::getDefaultSkinColor() };
		const QString r{ QString::number(backgroundColor.red()) };
		const QString g{ QString::number(backgroundColor.green()) };
		const QString b{ QString::number(backgroundColor.blue()) };
		const auto increaseValue{ 230 };

		qStyleSheet += QString("QWidget[titleskin=true] {\
									background-color: rgb(%1, %2, %3);\
									border-top-right-radius: 4px;\
								}\
								QWidget[bottomskin=true] {\
									background-color: rgb(%4, %5, %6);\
									border-top-right-radius: 4px;\
								}\
								QTreeView {\
									outline: none;\
									border-bottom-right-radius: 4px;\
									border-top: none\
								}\
								QTreeView::item{\
									color: rgba(255, 255, 255, 0);\
									outline: none;\
								}\
								QTreeView::item:selected:active{\
									background-color: rgba(%1, %2, %3, 153);\
									outline: none;\
								}\
								QTreeView::item:selected:active,\
								QTreeView::item:hover{\
									background-color: rgb(%4, %5, %6);\
									outline: none;\
								}").arg(r).arg(g).arg(b)
								   .arg(qMin(r.toInt() / 10 + increaseValue, 255))
								   .arg(qMin(g.toInt() / 10 + increaseValue, 255))
								   .arg(qMin(b.toInt() / 10 + increaseValue, 255));

		qStyleSheet += QString("QToolButton#closeBtn {\
									background-color: rgb(%1, %2, %3);\
									color: #ffffff;\
									font-size: 12px;\
									border-radius: 4px;\
								}\
								QToolButton#closeBtn:hover{\
								   background-color: rgba(%1, %2, %3, 150);\
								}\
								QToolButton#closeBtn:pressed{\
								   background-color: rgba(%1, %2, %3, 150);\
								}").arg(r).arg(g).arg(b);

		qStyleSheet += QString("QToolButton#sendBtn {\
									background-color: rgb(%1, %2, %3);\
									color: #ffffff;\
									font-size: 12px;\
									border-radius: 4px;\
								}\
								QToolButton#sendBtn:hover{\
									background-color: rgba(%1, %2, %3, 150);\
								}\
								QToolButton#sendBtn:pressed{\
									background-color: rgba(%1, %2, %3, 150);\
								}\
								QToolButton#sendBtn::menu-button{\
								    margin-top: 5px;\
								    border-top-right-radius: 4px;\
								    border-bottom-right-radius: 4px;\
								    border-left: 1px solid rgba(255, 255, 255, 100);\
								    background-color: transparent;\
								    width: 24px;\
								    height: 14px;\
								}\
								QToolButton#sendBtn::menu-arrow{\
									image: url(:Resources / MainWindow / aio_setting_white_normal.png);\
								}").arg(r).arg(g).arg(b);

		qStyleSheet += QString("QWidget#widget {\
									border-bottom-right-radius: 4px;\
								}\
								QTreeWidget#treeWidget {\
									border-bottom-right-radius: 4px;\
								}\
								QWidget#scrollAreaWidgetContents_2 {\
									border-bottom-right-radius: 4px;\
								}\
								QScrollArea#scrollArea {\
									border-bottom-right-radius: 4px;\
								}");


		setStyleSheet(qStyleSheet);
	}

	file.close();
}

void ChatWindow::dealMessage(
	QNChatMessage* messageW, QListWidgetItem* item, const QString& text, const QString& time,
	const QString& senderId, QNChatMessage::User_Type type) {
	//设置头像
	if (type == QNChatMessage::User_Me) {
		QPixmap headImage = CommonUtils::base64ToQPixmap(UserManager::getCurrentUser().getHeadImage());
		QPixmap mask{ ":/Resources/MainWindow/head_mask.png" };

		messageW->setRightHeadImage(CommonUtils::getRoundedImage(headImage, mask));
	} else if (type == QNChatMessage::User_She) {
		if (m_isGroupChat) {
			std::vector<GroupUser> currentGroupUsers{ UserManager::getGroup(m_chatId).getUsers() };
			const auto iter = std::ranges::find_if(currentGroupUsers, [senderId](const auto& groupUser) {
				return groupUser.getId() == senderId.toInt();
			});

			if (iter != std::end(currentGroupUsers)) {
				QPixmap headImage = CommonUtils::base64ToQPixmap((*iter).getHeadImage());
				QPixmap mask{ ":/Resources/MainWindow/head_mask.png" };

				messageW->setLeftHeadImage(CommonUtils::getRoundedImage(headImage, mask));
			}
		} else {
			QPixmap headImage = CommonUtils::base64ToQPixmap(UserManager::getFriend(m_chatId).getHeadImage());
			QPixmap mask{ ":/Resources/MainWindow/head_mask.png" };

			messageW->setLeftHeadImage(CommonUtils::getRoundedImage(headImage, mask));
		}
	}

	const auto width = ui.listWidget->size().width();
	messageW->setFixedWidth(width);
	const QSize size = messageW->fontRect(text);
	item->setSizeHint(QSize{ ui.listWidget->width(), size.height() });
	messageW->setText(text, time, size, type);

	//添加消息
	ui.listWidget->setItemWidget(item, messageW);
}

void ChatWindow::dealMessageTime(const QString& curMsgTime) {
	bool isShowTime = false;
	if (ui.listWidget->count() > 0) {
		QListWidgetItem* lastItem = ui.listWidget->item(ui.listWidget->count() - 1);
		auto* messageW = dynamic_cast<QNChatMessage*>(ui.listWidget->itemWidget(lastItem));
		const int lastTime = messageW->time().toInt();
		const int curTime = curMsgTime.toInt();

		// qDebug() << "curTime lastTime:" << curTime - lastTime;
		isShowTime = ((curTime - lastTime) > 60); // 两个消息相差一分钟
		//        isShowTime = true;
	} else {
		isShowTime = true;
	}
	if (isShowTime) {
		auto* messageTime = new QNChatMessage(ui.listWidget->parentWidget());
		auto* itemTime = new QListWidgetItem(ui.listWidget);

		const QSize size{ this->width(), 40 };
		messageTime->resize(size);
		itemTime->setSizeHint(QSize{ ui.listWidget->width(), size.height() });
		messageTime->setText(curMsgTime, curMsgTime, size, QNChatMessage::User_Time);
		ui.listWidget->setItemWidget(itemTime, messageTime);
	}
}

void ChatWindow::sendMessage(const QString& msg, const QString& time, const QString& senderId) {
	dealMessageTime(time);
	auto* messageW = new QNChatMessage(ui.listWidget->parentWidget());
	auto* item = new QListWidgetItem(ui.listWidget);
	dealMessage(messageW, item, msg, time, senderId, QNChatMessage::User_Me);

	if (m_isGroupChat) {
		int groupid = m_chatId.toInt();
		std::string message = msg.toStdString();

		nlohmann::json js;
		js["msgtype"] = static_cast<int>(MsgType::GROUP_CHAT_MSG);
		js["id"] = UserManager::getCurrentUser().getId();
		js["name"] = UserManager::getCurrentUser().getName();
		js["groupid"] = groupid;
		js["msg"] = message;
		js["time"] = time.toStdString();

		if (TcpClient::getInstance()->sendMessage(js, MsgType::GROUP_CHAT_MSG)) {
			messageW->setTextSuccess();
		}
	} else {
		int friendid = m_chatId.toInt();
		std::string message = msg.toStdString();

		nlohmann::json js;
		js["msgtype"] = static_cast<int>(MsgType::ONE_CHAT_MSG);
		js["id"] = UserManager::getCurrentUser().getId();
		js["name"] = UserManager::getCurrentUser().getName();
		js["toid"] = friendid;
		js["msg"] = message;
		js["time"] = time.toStdString();

		if (TcpClient::getInstance()->sendMessage(js, MsgType::ONE_CHAT_MSG)) {
			messageW->setTextSuccess();
		}
	}
}

void ChatWindow::onRecieveMessage(const QString msg, const QString time, const QString senderId) {
	dealMessageTime(time);
	auto* messageW = new QNChatMessage(ui.listWidget->parentWidget());
	auto* item = new QListWidgetItem(ui.listWidget);
	dealMessage(messageW, item, msg, time, senderId, QNChatMessage::User_She);
	messageW->setTextSuccess();
}

void ChatWindow::onSendBtnClicked(bool) {
	if (ui.textEdit->toPlainText().isEmpty()) {
		QToolTip::showText(this->mapToGlobal(QPoint(630, 660)), QString::fromLocal8Bit("发送的信息不能为空"), this,
		                   QRect(0, 0, 120, 100), 2000);
		return;
	}

	const QString msg = ui.textEdit->toPlainText();
	const QString time = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
	bool isSending = true; // 发送中

	sendMessage(msg, time, QString::number(UserManager::getCurrentUser().getId()));

	ui.listWidget->setCurrentRow(ui.listWidget->count() - 1);
	ui.textEdit->clear();
	// ui.textEdit->deleteAllEmojiImage();
	// ui.msgWidget->appendMsg(html); //收信息窗口
}

void ChatWindow::onItemDoubleClicked(QTreeWidgetItem* item, int colum) {
	if (const bool isChild = item->data(0, Qt::UserRole).toBool(); isChild) {
		QString peopleName = m_groupPeopleMap.value(item);
		// QString strEmployeeID = item->data(0, Qt::UserRole + 1).toString();
		// if (strEmployeeID == gLoginEmployeeID)
		// 	return;
		WindowManager::getInstance()->addNewChatWindow(item->data(0, Qt::UserRole + 1).toString(), false);
	}
}
