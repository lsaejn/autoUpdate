#include "SetupImageWidget.h"
#include <QLayout>
#include <QLabel>

SetupImageWidget::SetupImageWidget(QWidget* parent)
	:QWidget(parent)
{
	QVBoxLayout* vBox = new QVBoxLayout(this);
	for (int i = 0; i != 5; ++i)
		vBox->addWidget(new QLabel("fuck", this));
}