#pragma once

#include <QRect>
#include <QWidget>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QApplication>
#include <QDesktopWidget>
#else
#include <QGuiApplication>
#include <QScreen>
#endif

namespace QtComp
{
	namespace DesktopWidget
	{
		static QRect availableGeometry(const QWidget* widget = nullptr)
		{
	#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if (widget)
				return QApplication::desktop()->availableGeometry(widget);
			else
				return QApplication::desktop()->availableGeometry();
	#else
			QScreen* screen = nullptr;
			if (widget) {
				screen = QGuiApplication::screenAt(widget->geometry().center());
			}
			if (!screen)
				screen = QGuiApplication::primaryScreen();
			return screen ? screen->availableGeometry() : QRect();
	#endif
		}
	};
}
