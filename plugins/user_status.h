#ifndef USER_STATUS_H_
#define USER_STATUS_H_

struct user_status
{
	QString name;
	QString channel;
	QHostAddress ip;
	time_t lastseen;
};

#endif
