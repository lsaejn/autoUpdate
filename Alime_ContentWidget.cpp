#include "Alime_ContentWidget.h"


ContentWidgetCreator Alime_ContentWidget::creator_;

Alime_ContentWidget::Alime_ContentWidget(QWidget* parent)
	:QWidget(parent)
{

}

QString Alime_ContentWidget::GetTitle()
{
	return "Class Alime_ContentWidget";
}

QString Alime_ContentWidget::GetIcon()
{
	return ":/images/PkpmV52.ico";
}

int Alime_ContentWidget::GetShadowWidth()
{
	return 5;
}

QSize Alime_ContentWidget::GetWindowSize()
{
	return { 1000, 680 };
}