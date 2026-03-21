#pragma once

#include <QString>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#else
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#endif

namespace QtComp
{
	namespace RegExp
	{
		class QRegExp
		{
		public:
			QRegExp(const QString& pattern = QString(), bool caseInsensitive = false)
			{
				setPattern(pattern, caseInsensitive);
			}

			void setPattern(const QString& pattern, bool caseInsensitive = false)
			{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				m_re = QRegExp(pattern,
							   caseInsensitive ? Qt::CaseInsensitive : Qt::CaseSensitive);
		#else
				QRegularExpression::PatternOptions opts =
					caseInsensitive ? QRegularExpression::CaseInsensitiveOption
									: QRegularExpression::NoPatternOption;
				m_re.setPattern(pattern);
				m_re.setPatternOptions(opts);
		#endif
			}

			bool match(const QString& text)
			{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				m_lastIndex = m_re.indexIn(text);
				return m_lastIndex >= 0;
		#else
				m_match = m_re.match(text);
				return m_match.hasMatch();
		#endif
			}

			QString cap(int n) const
			{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return m_re.cap(n);
		#else
				return m_match.captured(n);
		#endif
			}

			QString captured(int n) const { return cap(n); }

			static QString escape(const QString& str)
			{
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return QRegExp::escape(str);
		#else
				return QRegularExpression::escape(str);
		#endif
			}

			   // Неявные конверсии — позволяют использовать в QString::contains()
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			operator QRegExp() const { return m_re; }
		#else
			operator QRegularExpression() const { return m_re; }
		#endif

		private:
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			QRegExp m_re;
			int m_lastIndex = -1;
		#else
			QRegularExpression m_re;
			QRegularExpressionMatch m_match;
		#endif
		};
	}
}
