#pragma once
#include <QPixmap>
#include <QSize>
#include <QProxyStyle>

//�ı�Ĭ�ϲ������
class CustomProxyStyle: public QProxyStyle{
public:
	CustomProxyStyle(QObject* parent) {
		setParent(parent);
	}

	virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const override {
		if(PE_FrameFocusRect == pe) {
			//ȥ��Windows��Ĭ�ϲ����ı߿�����ߣ�������ȡ����ʱֱ�ӷ��أ������л���
			return;
		} else {
			QProxyStyle::drawPrimitive(pe, opt, p, w);
		}
	}
};

class CommonUtils {
public:
	CommonUtils();
	~CommonUtils() = default;

public:
	static QPixmap getRoundedImage(const QPixmap& src, QPixmap& mask, QSize maskSize = QSize{ 0, 0 });
	static void loadStyleSheet(QWidget* widget, const QString& sheetName);
	static void setDefaultSkinColor(const QColor& color);
	static QColor getDefaultSkinColor();
};
