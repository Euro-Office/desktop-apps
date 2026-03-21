#pragma once

#include <QLayout>
#include <QPixmap>
#include <QLabel>
#include <QStyleOption>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
typedef long long_ptr;
#else
typedef qintptr long_ptr;
#endif

namespace QtComp
{
	namespace Widget
	{
		static void setLayoutMargin(QLayout* layout, const int& margin)
		{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			layout->setMargin(margin);
		#else
			layout->setContentsMargins(margin, margin, margin, margin);
		#endif
		}

		static QPixmap* copyPixmap(QLabel* label)
		{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			const QPixmap* pixmap = label->pixmap();
			if (pixmap) {
				return new QPixmap(*pixmap);
			}
		#else
			QPixmap pixmap = label->pixmap();
			if (!pixmap.isNull()) {
				return new QPixmap(pixmap);
			}
		#endif
			return nullptr;
		}

		static void initStyleOption(QStyleOption* option, const QWidget *w)
		{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			option->init(w);
		#else
			option->initFrom(w);
		#endif
		}
	}
}
