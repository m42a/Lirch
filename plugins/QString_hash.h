#ifndef QSTRING_HASH_
#define QSTRING_HASH_

#include <QString>

namespace std
{
	template <>
	struct hash<QString>
	{
		size_t operator()(const QString& v) const
		{
			return std::hash<std::string>()(v.toStdString());
		}
	};
}

#endif
