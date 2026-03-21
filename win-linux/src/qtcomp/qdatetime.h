#pragma once

#include <QDateTime>

namespace QtComp
{
	namespace DateTime
	{
		QDateTime fromTimestamp(qint64 timestamp)
		{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			return QDateTime::fromTime_t(static_cast<uint>(timestamp));
		#else
			return QDateTime::fromSecsSinceEpoch(timestamp);
		#endif
		}
	}
}
