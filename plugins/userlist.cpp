#include <thread>
#include <ctime>
#include <QSettings>
#include <iostream>

#include "lirch_plugin.h"
#include "userlist_messages.h"
#include "user_status.h"
#include "received_messages.h"
#include "notify_messages.h"
#include "grinder_messages.h"
#include "lirch_constants.h"
#include "nick_messages.h"
#include "channel_messages.h"
#include "blocker_messages.h"
#include "parser.h"

using namespace std;

message sendNick(QString str, QString)
{
	auto parsedNick=parse(str);
	if (parsedNick.size()<2 || parsedNick[0]!="/nick")
		return message::create<empty_message>();
	if (parsedNick.size()>2 && parsedNick[1]=="--")
		return message::create<nick_message>(parsedNick[2], false);
	if (parsedNick.size()>2 && parsedNick[1]=="-default")
		return message::create<nick_message>(parsedNick[2], true);
	return message::create<nick_message>(parsedNick[1], false);
}
message sendWhois(QString str, QString channel)
{
	if (str.startsWith("/whois "))
	{
		return message::create<who_is_message>(channel, str.section(' ',1));
	}
	return message::create<empty_message>();
}

struct userlist_timer
{
	static constexpr auto message_id="userlist_timer";
};

struct list_channels
{
	static constexpr auto message_id="list_channels";
	//list_channels(const QString &fch, const QString &dch) : filterChannel(fch), destinationChannel(dch) {}

	QString filterChannel;
	QString destinationChannel;
};

message sendList(QString text, QString channel)
{
	return message::create<list_channels>(text.section(QChar(' '), 1), channel);
}

//updates all of the relevent fields of out status map based on the received message
void updateSenderStatus(plugin_pipe p, message m, unordered_map<QString, user_status> & userList, QString currentNick)
{

	if (auto msg=m.try_extract<received_message>())
	{
		userList[msg->nick].nick=msg->nick;
		userList[msg->nick].channels.insert(msg->channel);
		userList[msg->nick].ip=msg->ipAddress;
		userList[msg->nick].lastseen=time(NULL);
	}

	if (auto msg=m.try_extract<received_status_message>())
	{
		userList[msg->nick].lastseen=time(NULL);
		userList[msg->nick].nick=msg->nick;
		userList[msg->nick].ip=msg->ipAddress;

		if (msg->subtype==received_status_message_subtype::LEFT)
		{
			userList[msg->nick].channels.erase(msg->channel);
			if (msg->channel=="")
			{
				for(const auto & iterator:userList[msg->nick].channels)
					p.write<notify_message>(iterator,msg->nick + " has logged off.");
				userList.erase(msg->nick);
			}
			else
				p.write<notify_message>(msg->channel,msg->nick+" has left channel "+msg->channel+".");
		}
		else if (msg->subtype==received_status_message_subtype::JOIN)
		{
			//Only notify people if they're joining a channel they're not in
			if (userList[msg->nick].channels.count(msg->channel)) {
				return;
			}
			userList[msg->nick].channels.insert(msg->channel);
			p.write<notify_message>(msg->channel,msg->nick+" has joined channel "+msg->channel+".");
		}
		else if (msg->subtype==received_status_message_subtype::HERE && msg->channel!="")
		{
			userList[msg->nick].channels.insert(msg->channel);
		}
		else if (msg->subtype==received_status_message_subtype::WHOHERE)
		{
			// Only reply that you're in channels you're in
			if (userList[currentNick].channels.count(msg->channel)) {
				p.write<here_message>(msg->channel);
				userList[msg->nick].channels.insert(msg->channel);
			}
		}
		else if (msg->subtype==received_status_message_subtype::NICK)
		{
			//message->channel is storing the old nickname of the user in the case that it is a nick type received.
			//if it is, remove the person of the old nickname
			user_status oldNickInfo = userList[msg->channel];
			oldNickInfo.nick=msg->nick;
			userList.erase(msg->channel);
			userList[msg->nick]=oldNickInfo;
			for(const auto & iterator:userList[msg->nick].channels)
				p.write<notify_message>(iterator,msg->channel+" has changed their nick to "+msg->nick+".");
		}

	}

	p.write<userlist_message>(currentNick, userList);
}

void askForUsers(plugin_pipe p, QString channel)
{
	for(int i=0; i<5; i++)
	{
		p.write<who_is_here_message>(channel);
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}

//validateName also sets old nick to new nick if it is acceptable
message setNick(const unordered_map<QString, user_status> &userList, QString &currentNick, nick_message &nick_msg, bool firstTime)
{
	// Case where we're trying to grab some else who isn't spartacus
	if (userList.count(nick_msg.nick) && nick_msg.nick!=LIRCH_DEFAULT_NICK)
	{
		if (firstTime)
			return message::create<notify_message>("",QObject::tr("Default nick taken.  You will be spartacus."));
		return message::create<notify_message>("",QObject::tr("Nick taken.  Keeping old nick."));
	}
	// Case when the nick is too long
	if (nick_msg.nick.toUtf8().size() > 64)
	{
		if (firstTime)
			return message::create<notify_message>("",QObject::tr("Default nick too long.  You will be spartacus."));
		return message::create<notify_message>("",QObject::tr("Nick too long.  Keeping old nick."));
	}
	// General case
	QString oldNick = currentNick;
	currentNick = nick_msg.nick;
	return message::create<changed_nick_message>(oldNick,currentNick,nick_msg.changeDefault);
}

void populateDefaultChannel(plugin_pipe p)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME);
	settings.beginGroup("UserData");
	QString defaultNick = settings.value("nick",LIRCH_DEFAULT_NICK).value<QString>();
	settings.sync();
	settings.endGroup();

	askForUsers(p,"default");

	p.write<nick_message>(defaultNick, true);

}

void run(plugin_pipe p, string name)
{
	p.write<registration_message>(0, name, "userlist_request");
	p.write<registration_message>(0, name, "userlist_timer");
	p.write<registration_message>(30000, name, "received");
	p.write<registration_message>(30000, name, "received_status");
	p.write<registration_message>(0, name, "list_channels");
	p.write<registration_message>(0, name, "handler_ready");
	p.write<registration_message>(0, name, "leave_channel");
	p.write<registration_message>(0, name, "set_channel");
	p.write<registration_message>(-30000, name, "nick");
	p.write<registration_message>(-30000, name, "who_is");
	p.write<registration_message>(-30000, name, "block name");

	bool firstTime=true;

	unordered_map<QString, user_status> userList;

	QString currentNick = LIRCH_DEFAULT_NICK;

	std::thread populate(populateDefaultChannel,p);
	populate.detach();


	while (true)
	{
		message m=p.blocking_read();
		if (m.is<shutdown_message>())
		{
			return;
		}
		else if (auto s=m.try_extract<registration_status>())
		{

			if (!s->status)
			{
				if ((0>=s->priority && s->priority>-200) || (30000>=s->priority && s->priority>29000))
					p.write<registration_message>(s->priority-1, name, s->type);
				else
					return;
			}
			else
			{
				if (s->type=="userlist_timer")
				{
					p.write<userlist_timer>();
				}
				else if (s->type=="handler_ready")
				{
					p.write<register_handler>("/list", sendList);
					p.write<register_handler>("/nick", sendNick);
					p.write<register_handler>("/whois", sendWhois);
				}
			}
		}
		else if (m.is<userlist_request>())//type=="userlist_request")
		{
			p.write<userlist_message>(currentNick, userList);
		}
		else if (m.is<userlist_timer>())//type=="userlist_timer")
		{
			time_t now=time(NULL);
			//Remove all nicks that haven't been seen in 2 minutes
			decltype(userList.begin()) i;
			while ((i=std::find_if(userList.begin(), userList.end(), [now](const std::pair<const QString &, const user_status &> &p) {return p.second.lastseen<now-2*60;}))!=userList.end())
			{
				for(auto iter = i->second.channels.begin(); iter!=i->second.channels.end(); iter++)
					p.write<notify_message>(*iter, i->first + " has logged off.");
				userList.erase(i);
			}
			p.write<userlist_message>(currentNick, userList);
			thread([](plugin_pipe p)
					{
					this_thread::sleep_for(chrono::seconds(10));
					p.write<userlist_timer>();
					}, p).detach();
		}
		else if (m.is<handler_ready>())//type=="handler_ready")
		{
			p.write<register_handler>("/list", sendList);
			p.write<register_handler>("/nick", sendNick);
			p.write<register_handler>("/whois", sendWhois);
		}
		else if (m.is<received_message>() || m.is<received_status_message>())//.type=="received" || m.type=="received_status")
		{
			p.write(m.decrement_priority());
			updateSenderStatus(p,m,userList,currentNick);
		}
		else if (auto s=m.try_extract<nick_message>())
		{
			p.write(m.decrement_priority());
			if (firstTime)
				p.write<set_channel_message>("default");
			p.write(setNick(userList,currentNick,*s,firstTime));

			//The userlist is no longer a virgin
			firstTime = false;

			if (s->changeDefault)
			{
				QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME);
				settings.beginGroup("UserData");
				settings.setValue("nick", currentNick);
				settings.sync();
				settings.endGroup();
			}
		}
		else if (auto s=m.try_extract<list_channels>())
		{
			for (auto &i : userList)
			{
				if (s->filterChannel=="" || i.second.channels.count(s->filterChannel)!=0)
				{
					QStringList channelList;
					for (auto &c : i.second.channels)
						channelList.append(c);
					p.write<notify_message>(s->destinationChannel, QObject::tr("User %1 (%2) was last seen at %3 and is in the following channels: %4").arg(i.second.nick, i.second.ip.toString(), QDateTime::fromTime_t(i.second.lastseen).toString(), channelList.join(" ")));
				}
			}
		}
		else if (auto s=m.try_extract<set_channel_message>())
		{
			p.write(m.decrement_priority());

			// If you can't in the channel yet, ask who's there
			if (userList[currentNick].channels.count(s->channel)==0)
				askForUsers(p,s->channel);
		}
		else if (auto s=m.try_extract<leave_channel_message>())
		{
			p.write(m.decrement_priority());

			for(auto & person:userList)
			{
				person.second.channels.erase(s->channel);
			}
		}
		else if (auto s=m.try_extract<who_is_message>())
		{
			p.write(m.decrement_priority());

			if (userList.count(s->nick)==0)
				continue;

			QStringList channelList;
			for (auto &c : userList[s->nick].channels)
				channelList.append(c);

			p.write<notify_message>(s->channel, QObject::tr("User %1 (%2) was last seen at %3 and is in the following channels: %4").arg(userList[s->nick].nick, userList[s->nick].ip.toString(), QDateTime::fromTime_t(userList[s->nick].lastseen).toString(), channelList.join(" ")));
		}
		else
			p.write(m.decrement_priority());
	}
}


