#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>

enum class ButtonType {
	MIN_BUTTON = 0,
	//��С�����رհ�ť

	MIN_MAX_BUTTON,
	//��С������󻯼��رհ�ť

	ONLY_CLOSE_BUTTON,
	//���رհ�ť
};

//�Զ��������
class TitleBar : public QWidget {
	Q_OBJECT

public:
	TitleBar(QWidget* parent = nullptr);
	~TitleBar();

	void setTitleIcon(const QString& filePath);
	void setTitleContent(const QString& titleContent);
	void setTitleWidth(int width);
	void setButtonType(ButtonType buttonType);

	//����&��ȡ���ǰ����λ�ü���С
	void saveRestoreInfo(const QPoint& point, const QSize& size);
	void getRestoreInfo(QPoint& point, QSize& size);

private:
	void paintEvent(QPaintEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	//��ʼ���ؼ�
	void initControl();
	//��ʼ��signal-slot
	void initConnections();
	//������ʽ��
	void loadStyleSheet(const QString& sheetName);

signals:
	//��С����ť
	void signalButtonMinClicked();
	//��󻯻�ԭ��ť
	void signalButtonRestoreClicked();
	//��󻯰�ť
	void signalButtonMaxClicked();
	//�رհ�ť
	void signalButtonCloseClicked();

private slots:
	void onButtonMinClicked();
	void onButtonRestoreClicked();
	void onButtonMaxClicked();
	void onButtonCloseClicked();

private:
	//������ͼ��
	QLabel* m_pIcon{};
	//����������
	QLabel* m_ptitleContent{};
	QPushButton* m_pButtonMin{};
	QPushButton* m_pButtonRestore{};
	QPushButton* m_pButtonMax{};
	QPushButton* m_pButtonClose{};

	//��󻯻�ԭ��ť
	QPoint m_restorePos;
	QSize m_restoreSize;

	//�Ƿ��ƶ�
	bool m_isPressed;
	QPoint m_startMovePos;

	//����������
	QString m_titleContent;

	//��������ť����
	ButtonType m_buttonType;
};
