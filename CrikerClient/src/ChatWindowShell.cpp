﻿#include "ChatWindowShell.h"

#include <QFile>
#include <QMessageBox>
#include <QSqlQueryModel>

#include "CommonUtils.h"
#include "UserManager.h"

ChatWindowShell::ChatWindowShell(QWidget* parent)
	: BasicWindow{ parent } {
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);
	initControl();

	//获取用户当前皮肤的RGB
	QColor backgroundColor{ CommonUtils::getDefaultSkinColor() };
	const QString r{ QString::number(backgroundColor.red()) };
	const QString g{ QString::number(backgroundColor.green()) };
	const QString b{ QString::number(backgroundColor.blue()) };
	const auto increaseValue{ 230 };

	QString qss = QString(
		"QListView#shellListWidget {"
		"    padding: 4px; "
		"    border-bottom-left-radius: 4px; "
		"    border-top-left-radius: 4px; "
		"}"
		"QListView#shellListWidget::item {"
		"    border-radius: 4px;"
		"    background-color: transparent;"
		"    height: 60px;"
		"}"
		"QListView#shellListWidget::item:selected {"
		"    border-radius: 4px;"
		"    background-color: rgba(%1, %2, %3, %4);"
		"}"
		"QListView#shellListWidget::item:selected:!active {"
		"    border-radius: 4px;"
		"    background-color: rgba(%1, %2, %3, %4);"
		"}"
		"QListView#shellListWidget::item:selected:active {"
		"    border-radius: 4px;"
		"    background-color: rgba(%1, %2, %3, %4);"
		"}"
		"QListView#shellListWidget::item:hover {"
		"    background-color: rgba(%1, %2, %3, %5);"
		"}").arg(qMin(r.toInt() / 10 + increaseValue, 255))
		.arg(qMin(g.toInt() / 10 + increaseValue, 255))
		.arg(qMin(b.toInt() / 10 + increaseValue, 255))
		.arg(150).arg(100);

	qss += QString(
		"QListView#shellListWidget {"
		"    background-color: rgb(%1, %2, %3);"
		"}"
		"QStackedWidget#rightStackedWidget {"
		"    background-color: transparent;"
		"    border-top-right-radius: 4px;"
		"    border-bottom-right-radius: 4px;"
		"}"
		"QWidget#ChatWindowShellClass {"
		"    border-radius: 4px;"
		"    background-color: white;"
		"    border: 1px solid rgb(%1, %2, %3);"
		"}").arg(r).arg(g).arg(b);

	setStyleSheet(qss);



	// ui.listWidget->setStyleSheet(QString{ "QListView {\
	// 										background-color: rgb(%1, %2, %3);\
	// 									 }" }.arg(r).arg(g).arg(b));
	//
	// ui.listWidget->setStyleSheet(QString{ "QListView {\
	// 										padding: 4px; \
	// 										border: none;\
	// 									 }\
	// 									 QListView:item {\
	// 										border-radius:4px;\
	// 										background-color: transparent;\
	// 										height: 60px;\
	// 									 }\
	// 									 QListView::item:selected {\
	// 										border-radius:4px;\
	// 										background-color: rgba(%1, %2, %3, %4);\
	// 									 }\
	// 									 QListView::item:selected:!active{\
	// 										border-radius:4px;\
	// 										background-color: rgba(%1, %2, %3, %4);\
	// 									 }\
	// 									 QListView::item:selected:active{\
	// 										border-radius:4px;\
	// 										background-color: rgba(%1, %2, %3, %4);\
	// 									 }\
	// 									 QListView::item:hover{\
	// 										background-color: rgba(%1, %2, %3, %5);\
	// 									 }" }.arg(qMin(r.toInt() / 10 + increaseValue, 255))
	// 									     .arg(qMin(g.toInt() / 10 + increaseValue, 255))
	// 									     .arg(qMin(b.toInt() / 10 + increaseValue, 255))
	// 									     .arg(150).arg(100)
	// );
	//
	// setStyleSheet(QString{ "QWidget#ChatWindowShellClass {\
	// 								border-radius: 4px;\
	// 								border: 1px solid #000000;\
	// 							}" });
}

ChatWindowShell::~ChatWindowShell() {
	delete m_emojiWindow;
}

void ChatWindowShell::addChatWindow(
	ChatWindow* chatWindow, ChatWindowItem* chatWindowItem, const QString& uid, bool isGroupChat) {
	ui.rightStackedWidget->addWidget(chatWindow);

	connect(m_emojiWindow, SIGNAL(signalEmojiWindowHide()), chatWindow, SLOT(onSetEmojiBtnFocusStatus()));

	QListWidgetItem* item{ new QListWidgetItem{ ui.shellListWidget } };
	item->setSelected(true);
	m_chatWindowItemMap.insert(item, chatWindow);

	// //判断是单聊还是群聊，获取头像
	// QSqlQueryModel sqlDepModel;
	// QString strSql = QString("SELECT picture FROM tab_department WHERE departmentID=%1").arg(uid);
	// sqlDepModel.setQuery(strSql);
	// int rows = sqlDepModel.rowCount();
	//
	// //单聊
	// if (rows == 0) {
	// 	QString sql = QString("SELECT picture FROM tab_employees WHERE employeeID=%1").arg(uid);
	// 	sqlDepModel.setQuery(sql);
	// }
	// QModelIndex index = sqlDepModel.index(0, 0);
	//
	// QImage img;
	// img.load(sqlDepModel.data(index).toString());
	// chatWindowItem->setHeadPixmap(QPixmap::fromImage(img)); //设置头像

	ui.shellListWidget->addItem(item);
	ui.shellListWidget->setItemWidget(item, chatWindowItem);

	onChatWindowItemClicked(item);
	connect(chatWindowItem, &ChatWindowItem::signalCloseClicked,
	        [chatWindowItem, chatWindow, item, this]() {
		        m_chatWindowItemMap.remove(item);
		        chatWindow->close();
		        ui.shellListWidget->takeItem(ui.shellListWidget->row(item));
		        delete chatWindowItem;
		        ui.rightStackedWidget->removeWidget(chatWindow);

		        if (ui.rightStackedWidget->count() < 1) {
			        close();
		        }
	        });
}

void ChatWindowShell::setCurrentWidget(QWidget* widget) const {
	ui.rightStackedWidget->setCurrentWidget(widget);
}

QMap<QListWidgetItem*, QWidget*> ChatWindowShell::getChatWindowItemMap() const {
	return m_chatWindowItemMap;
}

bool ChatWindowShell::eventFilter(QObject* obj, QEvent* event) {
	if (obj == m_emojiWindow && event->type() == QEvent::Leave) {
		// 鼠标离开Emoji面板时隐藏面板
		m_emojiWindow->hide();
		// return true;
	}
	return QDialog::eventFilter(obj, event);
}

void ChatWindowShell::onEmojiBtnClicked(bool) {
	m_emojiWindow->setVisible(!m_emojiWindow->isVisible());
	QPoint emojiPoint = this->mapToGlobal(QPoint{ 0, 0 }); //将当前控件的相对位置转换为屏幕的绝对位置

	emojiPoint.setX(emojiPoint.x() + 170);
	emojiPoint.setY(emojiPoint.y() + 220);

	m_emojiWindow->move(emojiPoint);

	// 安装事件过滤器，以捕获鼠标离开事件
	m_emojiWindow->installEventFilter(this);
}

void ChatWindowShell::onChatWindowItemClicked(QListWidgetItem* item) {
	QWidget* chatWindowWidget{ m_chatWindowItemMap.find(item).value() };
	ui.rightStackedWidget->setCurrentWidget(chatWindowWidget);
}

void ChatWindowShell::onEmojiItemClicked(QString unicodeEmoji) const {
	ChatWindow* currentChatWindow{ dynamic_cast<ChatWindow*>(ui.rightStackedWidget->currentWidget()) };
	if (currentChatWindow != nullptr) {
		currentChatWindow->addEmojiImage(unicodeEmoji);
	}
}

void ChatWindowShell::initControl() {
	loadStyleSheet("TalkWindow");
	setWindowTitle(QString{ "聊天窗口" });

	m_emojiWindow = new EmojiWindow{};
	m_emojiWindow->hide();

	QList<int> leftWidgetSize;
	leftWidgetSize.append(180);
	leftWidgetSize.append(width() - 180);
	ui.splitter->setSizes(leftWidgetSize);

	ui.shellListWidget->setStyle(new CustomProxyStyle{ this });

	connect(ui.shellListWidget, &QListWidget::itemClicked, this, &ChatWindowShell::onChatWindowItemClicked);
	connect(m_emojiWindow, SIGNAL(signalEmojiItemClicked(QString)), this, SLOT(onEmojiItemClicked(QString)));
}

QStringList ChatWindowShell::getEmployeesID() {
	QSqlQueryModel queryModel;
	queryModel.setQuery("SELECT employeeID FROM tab_employees WHERE `status`=1");
	QStringList employeeIDList;

	//员工总数
	int employeesNum = queryModel.rowCount();
	QModelIndex index;
	for (int i = 0; i < employeesNum; i++) {
		index = queryModel.index(i, 0);
		employeeIDList << queryModel.data(index).toString();
	}
	return employeeIDList;
}
