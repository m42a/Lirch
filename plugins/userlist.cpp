#include <thread>
#include <ctime>

#include "lirch_plugin.h"
#include "userlist_messages.h"
#include "user_status.h"
#include "received_messages.h"
#include "notify_messages.h"
#include "grinder_messages.h"
#include "lirch_constants.h"

using namespace std;

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
void updateSenderStatus(received_message * message, unordered_map<QString, user_status> & statuses)
{
	statuses[message->nick].nick=message->nick;
	if (message->subtype==received_message_subtype::NORMAL || message->subtype==received_message_subtype::ME || message->subtype==received_message_subtype::NOTIFY)
		statuses[message->nick].channels.insert(message->channel);
	statuses[message->nick].ip=message->ipAddress;
	statuses[message->nick].lastseen=time(NULL);
}

void askForUsers(plugin_pipe p, QString channel)
{
	time_t start = time(NULL);
	while (time(NULL)-start < 60)
	{
		p.write(who_is_here_message::create(channel));
		this_thread::sleep_for(chrono::seconds(2));
	}
	return;
}

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(0, name, "userlist_request"));
	p.write(registration_message::create(0, name, "userlist_timer"));
	p.write(registration_message::create(30000, name, "received"));
	p.write(registration_message::create(30000, name, "received_me"));
	p.write(registration_message::create(0, name, "list_channels"));
	p.write(registration_message::create(0, name, "handler_ready"));
	unordered_map<QString, user_status> statuses;

	thread startupPopulator(askForUsers,p,LIRCH_DEFAULT_CHANNEL);
	startupPopulator.detach();
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
			p.write(userlist_message::create(statuses));
		}
		else if (m.type=="userlist_timer")
		{
			time_t now=time(NULL);
			//Remove all nicks that haven't been seen in 2 minutes
			decltype(statuses.begin()) i;
			while ((i=std::find_if(statuses.begin(), statuses.end(), [now](const std::pair<const QString &, const user_status &> &p) {return p.second.lastseen<now-2*60;}))!=statuses.end())
				statuses.erase(i);
			p.write(userlist_message::create(statuses));
			thread([](plugin_pipe p)
			{
				this_thread::sleep_for(chrono::seconds(10));
				p.write(userlist_timer::create());
			}, p).detach();
		}
		else if (m.type=="handler_ready")
		{
			p.write(register_handler::create("/list", sendList));
		}
		else if (m.type=="received")
		{
			auto s=dynamic_cast<received_message *>(m.getdata());
			if (!s)
				continue;
			p.write(m.decrement_priority());
			updateSenderStatus(s,statuses);
		}
		else if (m.type=="list_channels")
		{
			auto s=dynamic_cast<list_channels *>(m.getdata());
			if (!s)
				continue;
			for (auto &i : statuses)
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


