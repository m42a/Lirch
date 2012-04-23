#ifndef QHOSTADDRESS_H_
#define QHOSTADDRESS_H_

#include <QHostAddress>

#include "QString_hash.h"

namespace std
{
	template <>
	struct hash<QHostAddress>
	{
		size_t operator()(const QHostAddress& v) const
		{
			return std::hash<QString>()(v.toString());
		}
	};
}

#endif
