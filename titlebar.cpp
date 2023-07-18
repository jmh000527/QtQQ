#include "titlebar.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QFile>

enum {
	BUTTON_HEIGHT = 27,
	BUTTON_WIDTH = 27,
	TITLE_HEIGHT = 27
};

TitleBar::TitleBar(QWidget* parent)
	: QWidget(parent), m_isPressed{ false }, m_buttonType{ ButtonType::MIN_MAX_BUTTON } {
	initControl();
	initConnections();
	loadStyleSheet("Title");
}

TitleBar::~TitleBar() {}

void TitleBar::setTitleIcon(const QString& filePath) {
	QPixmap titleIcon{ filePath };
	m_pIcon->setFixedSize(titleIcon.size());
	m_pIcon->setPixmap(titleIcon);
}

void TitleBar::setTitleContent(const QString& titleContent) {
	m_ptitleContent->setText(titleContent);
	m_titleContent = titleContent;
}

void TitleBar::setTitleWidth(int width) {
	setFixedWidth(width);
}

void TitleBar::setButtonType(ButtonType buttonType) {
	m_buttonType = buttonType;

	switch (buttonType) {
		case ButtonType::MIN_BUTTON: {
			m_pButtonRestore->setVisible(false);
			m_pButtonMax->setVisible(false);

			break;
		}
		case ButtonType::MIN_MAX_BUTTON: {
			m_pButtonRestore->setVisible(false);

			break;
		}
		case ButtonType::ONLY_CLOSE_BUTTON: {
			m_pButtonRestore->setVisible(false);
			m_pButtonMax->setVisible(false);
			m_pButtonMin->setVisible(false);

			break;
		}
		default: {
			break;
		}
	}
}

void TitleBar::saveRestoreInfo(const QPoint& point, const QSize& size) {
	m_restorePos = point;
	m_restoreSize = size;
}

void TitleBar::getRestoreInfo(QPoint& point, QSize& size) {
	point = m_restorePos;
	size = m_restoreSize;
}

void TitleBar::paintEvent(QPaintEvent* event) {
	//���ñ���ɫ
	QPainter painter{ this };
	QPainterPath pathBack;
	pathBack.setFillRule(Qt::WindingFill);	//����������
	pathBack.addRoundedRect(QRect{ 0, 0, this->width(), this->height() }, 3, 3);	//���Բ�Ǿ��ε���ͼ·��
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	//��������󻯻�ԭ�󣬴��ڳ��ȸı䣬������������Ӧ�ı�
	//parentWidget()���ظ�����
	if(this->width() != parentWidget()->width()) {
		setFixedWidth(parentWidget()->width());
	}

	return QWidget::paintEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
	if(m_buttonType == ButtonType::MIN_MAX_BUTTON) {
		if(m_pButtonMax->isVisible()) {
			onButtonMaxClicked();
		} else {
			onButtonRestoreClicked();
		}
	}

	return QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::mousePressEvent(QMouseEvent* event) {
	if(m_buttonType == ButtonType::MIN_MAX_BUTTON) {
		//�����ʱ��ֹ�϶�����
		if(m_pButtonMax->isVisible()) {
			m_isPressed = true;
			m_startMovePos = event->globalPos();	//�����¼�����ʱ����ȫ��λ��
		}
	} else {
		m_isPressed = true;
		m_startMovePos = event->globalPos();
	}

	return QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent* event) {
	if(m_isPressed) {
		QPoint movePoint{ event->globalPos() - m_startMovePos };
		QPoint widgetPos{ parentWidget()->pos() };

		m_startMovePos = event->globalPos();
		parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
	}

	return QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event) {
	m_isPressed = false;

	return QWidget::mouseReleaseEvent(event);
}

void TitleBar::initControl() {
	m_pIcon = new QLabel{ this };
	m_ptitleContent = new QLabel{ this };

	m_pButtonMin = new QPushButton{ this };
	m_pButtonRestore = new QPushButton{ this };
	m_pButtonMax = new QPushButton{ this };
	m_pButtonClose = new QPushButton{ this };

	//���ð�ť�̶���С
	m_pButtonMin->setFixedSize(QSize{ BUTTON_WIDTH, BUTTON_HEIGHT });
	m_pButtonRestore->setFixedSize(QSize{ BUTTON_WIDTH, BUTTON_HEIGHT });
	m_pButtonMax->setFixedSize(QSize{ BUTTON_WIDTH, BUTTON_HEIGHT });
	m_pButtonClose->setFixedSize(QSize{ BUTTON_WIDTH, BUTTON_HEIGHT });

	//���ö�����
	m_ptitleContent->setObjectName("TiitleContent");
	m_pButtonMin->setObjectName("ButtonMin");
	m_pButtonRestore->setObjectName("ButtonRestore");
	m_pButtonMax->setObjectName("ButtonMax");
	m_pButtonClose->setObjectName("ButtonClose");

	//���ò���
	auto* myLayout{ new QHBoxLayout{ this } };
	myLayout->addWidget(m_pIcon);
	myLayout->addWidget(m_ptitleContent);

	myLayout->addWidget(m_pButtonMin);
	myLayout->addWidget(m_pButtonRestore);
	myLayout->addWidget(m_pButtonMax);
	myLayout->addWidget(m_pButtonClose);

	myLayout->setContentsMargins(5, 0, 0, 0); //�����ܱߵļ�϶
	myLayout->setSpacing(0); //�����в���֮��ļ�϶

	m_ptitleContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(TITLE_HEIGHT);
	setWindowFlags(Qt::FramelessWindowHint);
}

void TitleBar::initConnections() {
	connect(m_pButtonMin, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
	connect(m_pButtonRestore, SIGNAL(clicked()), this, SLOT(onButtonRestoreClicked()));
	connect(m_pButtonMax, SIGNAL(clicked()), this, SLOT(onButtonMaxClicked()));
	connect(m_pButtonClose, SIGNAL(clicked()), this, SLOT(onButtonCloseClicked()));
}

void TitleBar::loadStyleSheet(const QString& sheetName) {
	QFile file{ ":/Resources/QSS" + sheetName + ".css" };

	file.open(QFile::ReadOnly);
	if(file.isOpen()) {
		QString styleSheet{ this->styleSheet() };
		styleSheet += QLatin1String{file.readAll()};
		setStyleSheet(styleSheet);
	}
}

void TitleBar::onButtonMinClicked() {
	emit signalButtonMinClicked();
}

void TitleBar::onButtonRestoreClicked() {
	m_pButtonRestore->setVisible(false);
	m_pButtonMax->setVisible(true);

	emit signalButtonRestoreClicked();
}

void TitleBar::onButtonMaxClicked() {
	m_pButtonMax->setVisible(false);
	m_pButtonRestore->setVisible(true);

	emit signalButtonMaxClicked();
}

void TitleBar::onButtonCloseClicked() {
	emit signalButtonCloseClicked();
}
