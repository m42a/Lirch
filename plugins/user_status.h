#ifndef USER_STATUS_H_
#define USER_STATUS_H_

#include <QCore/QString>
#include <QNetwork>
#include <ctime>
#include <unordered_set>

namespace std
{
	template <>
	struct hash<QString>
	{
		size_t operator()(const QString& v) const
		{
			//This isn't unique, but hashes need only conflict rarely, not never
			return std::hash<std::string>()(v.toStdString());
		}
	};

}

struct user_status
{
	QString nick;
	unordered_set<QString> channels;
	QHostAddress ip;
	time_t lastseen;
};

#endif
