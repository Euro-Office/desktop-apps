#pragma once

#include <QPalette>

namespace QtComp
{
	namespace Palette
	{
	#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		constexpr QPalette::ColorRole Background = QPalette::Background;
	#else
		constexpr QPalette::ColorRole Background = QPalette::Window;
	#endif
	}
}
