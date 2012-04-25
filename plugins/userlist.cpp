#include <thread>
#include <ctime>
#include <QSettings>

#include "lirch_plugin.h"
#include "userlist_messages.h"
#include "user_status.h"
#include "received_messages.h"
#include "notify_messages.h"
#include "grinder_messages.h"
#include "lirch_constants.h"
#include "nick_messages.h"
#include "channel_messages.h"

using namespace std;

message sendNick(QString str, QString)
{
	if (str.startsWith("/nick "))
	{
		return nick_message::create(str.section(' ',1));
	}
	return empty_message::create();
}

class userlist_timer : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(nullptr);}
	static message create() {return message_create("userlist_timer", nullptr);}
};

class list_channels : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new list_channels(*this));}
	static message create(const QString &fch, const QString &dch) {return message_create("list_channels", new list_channels(fch,dch));}
	list_channels(const QString &fch, const QString &dch) : filterChannel(fch), destinationChannel(dch) {}

	QString filterChannel;
	QString destinationChannel;
};

message sendList(QString text, QString channel)
{
	return list_channels::create(text.section(QChar(' '), 1), channel);
}

//updates all of the relevent fields of out status map based on the received message
void updateSenderStatus(plugin_pipe p, message m, unordered_map<QString, user_status> & userList, QString currentNick)
{

	if (m.type=="received")
	{
		auto message=dynamic_cast<received_message *>(m.getdata());
		userList[message->nick].nick=message->nick;
		userList[message->nick].channels.insert(message->channel);
		userList[message->nick].ip=message->ipAddress;
		userList[message->nick].lastseen=time(NULL);
	}

	if (m.type=="received_status")
	{
		auto message=dynamic_cast<received_status_message *>(m.getdata());
		userList[message->nick].lastseen=time(NULL);
		userList[message->nick].nick=message->nick;
		userList[message->nick].ip=message->ipAddress;

		if (message->subtype==received_status_message_subtype::LEFT)
		{
			userList[message->nick].channels.erase(message->channel);
			p.write(notify_message::create(message->channel,message->nick+" has left channel "+message->channel+"."));
		}
		else if (message->subtype==received_status_message_subtype::HERE)
		{
			userList[message->nick].channels.insert(message->channel);
		}
		else if (message->subtype==received_status_message_subtype::WHOHERE)
		{
			p.write(here_message::create(message->channel));
			userList[message->nick].channels.insert(message->channel);
		}
		else if (message->subtype==received_status_message_subtype::NICK)
		{
			//message->channel is storing the old nickname of the user in the case that it is a nick type received.
			//if it is, remove the person of the old nickname
			user_status oldNickInfo = userList[message->channel];
			oldNickInfo.nick=message->nick;
			userList.erase(message->channel);
			userList[message->nick]=oldNickInfo;
		}

	}

	p.write(userlist_message::create(currentNick, userList));
}

void askForUsers(plugin_pipe p, QString channel)
{
	time_t start = time(NULL);
	while (time(NULL)-start < 1)
	{
		p.write(who_is_here_message::create(channel));
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}

//validateName also sets old nick to new nick if it is acceptable
bool setNick(plugin_pipe p, unordered_map<QString, user_status> & userList,QString & oldNick, QString newNick)
{
	if (newNick!=LIRCH_DEFAULT_NICK && userList.count(newNick))
		return false;
	else
	{
		p.write(changed_nick_message::create(oldNick,newNick));
		oldNick = newNick;
		return true;
	}
}

void populateDefaultChannel(plugin_pipe p, QString channel, unordered_map<QString, user_status> & userList,QString & nick)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME);
	settings.beginGroup("UserData");
	QString defaultNick = settings.value("nick",LIRCH_DEFAULT_NICK).value<QString>();
	settings.sync();
	settings.endGroup();

	askForUsers(p,channel);

	if (defaultNick.toUtf8().size() > 64)
	{
		p.write(notify_message::create("","Default nick too long.  You are spartacus."));
		return;
	}

	if (setNick(p,userList,nick,defaultNick))
		p.write(notify_message::create(channel,"Welcome to LIRCH, "+defaultNick+"."));
	else
		p.write(notify_message::create(channel,"Default nick taken.  You can assign a new one with /nick <username>"));

}

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(0, name, "userlist_request"));
	p.write(registration_message::create(0, name, "userlist_timer"));
	p.write(registration_message::create(30000, name, "received"));
	p.write(registration_message::create(30000, name, "received_status"));
	p.write(registration_message::create(0, name, "list_channels"));
	p.write(registration_message::create(0, name, "handler_ready"));
	p.write(registration_message::create(0, name, "leave_channel"));
	p.write(registration_message::create(-30000, name, "nick"));


	unordered_map<QString, user_status> userList;

	QString currentNick = LIRCH_DEFAULT_NICK;

	populateDefaultChannel(p,LIRCH_DEFAULT_CHANNEL,userList,currentNick);

	p.write(register_handler::create("/nick", sendNick));

	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			return;
		}
		else if (m.type=="registration_status")
		{
			auto s=dynamic_cast<registration_status *>(m.getdata());
			if (!s)
				continue;
			if (!s->status)
			{
				if ((0>=s->priority && s->priority>-200) || (30000>=s->priority && s->priority>29000))
					p.write(registration_message::create(s->priority-1, name, s->type));
				else
					return;
			}
			else
			{
				if (s->type=="userlist_timer")
				{
					p.write(userlist_timer::create());
				}
				else if (s->type=="list_channels")
				{
					p.write(register_handler::create("/list", sendList));
				}
			}
		}
		else if (m.type=="userlist_request")
		{
			p.write(userlist_message::create(currentNick, userList));
		}
		else if (m.type=="userlist_timer")
		{
			time_t now=time(NULL);
			//Remove all nicks that haven't been seen in 2 minutes
			decltype(userList.begin()) i;
			while ((i=std::find_if(userList.begin(), userList.end(), [now](const std::pair<const QString &, const user_status &> &p) {return p.second.lastseen<now-2*60;}))!=userList.end())
			{
				for(auto iter = i->second.channels.begin(); iter!=i->second.channels.end(); iter++)
					p.write(notify_message::create(*iter, i->first.repeated(1).append(" has logged off")));
				userList.erase(i);
			}
			p.write(userlist_message::create(currentNick, userList));
			thread([](plugin_pipe p)
			{
				this_thread::sleep_for(chrono::seconds(10));
				p.write(userlist_timer::create());
			}, p).detach();
		}
		else if (m.type=="handler_ready")
		{
			p.write(register_handler::create("/list", sendList));
			p.write(register_handler::create("/nick", sendNick));
		}
		else if (m.type=="received" || m.type=="received_status")
		{
			p.write(m.decrement_priority());
			updateSenderStatus(p,m,userList,currentNick);
		}
		else if (m.type=="nick")
		{
			auto s=dynamic_cast<nick_message *>(m.getdata());
			if (!s)
				continue;
			p.write(m.decrement_priority());
			if (s->nick.toUtf8().size() > 64)
			{
				p.write(notify_message::create("","Nick too long.  Keeping old nick."));
				continue;
			}
			QString newNick=s->nick;
			QString oldNick=currentNick;
			if (setNick(p,userList,currentNick,newNick))
				p.write(sendable_notify_message::create("",oldNick+" has changed their nick to "+newNick+"."));
			else
				p.write(notify_message::create("","Nick taken.  Keeping old nick."));
		}
		else if (m.type=="list_channels")
		{
			auto s=dynamic_cast<list_channels *>(m.getdata());
			if (!s)
				continue;
			for (auto &i : userList)
			{
				if (s->filterChannel=="" || i.second.channels.count(s->filterChannel)!=0)
				{
					QStringList channelList;
					for (auto &c : i.second.channels)
						channelList.append(c);
					p.write(notify_message::create(s->destinationChannel, QObject::tr("User %1 (%2) was last seen at %3 and is in the following channels: %4").arg(i.second.nick, i.second.ip.toString(), ctime(&i.second.lastseen), channelList.join(" "))));
				}
			}
		}
		else
			p.write(m.decrement_priority());
	}
}


