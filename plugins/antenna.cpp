/*
 * IP address for the multicast group 224.0.0.224
 * Port for multicast group 45454
 *
 *
 * broadcasts are of the format
 * [type][channel][nick][contents]
 * type is a 4 byte string, currently "edct" for normal edicts, "mdct" for medicts, "ntfy" for notifies.
 * channel is a 64 byte string which contains the destination channel for the message terminated with zero characters.
 * nick is the same size and idea as channel, except it contains the nick of the sender.
 * contents is a max 256 byte string of whatever the text being sent is.  If the contents are shorter, the broadcast is shorter to match.
 *
 *
 * To Do:
 * make sure the nick/channel/message length being broadcast is short enough
 * toss a notify to the UI if ^ fails
 */


#include <thread>
#include <iostream>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QSettings>
#include <QtNetwork>
#include <unordered_set>

#include "lirch_constants.h"
#include "blocker_messages.h"
#include "edict_messages.h"
#include "received_messages.h"
#include "lirch_plugin.h"
#include "lirch_constants.h"
#include "grinder_messages.h"
#include "notify_messages.h"

using namespace std;


//this nonsense is needed in order to have our blocklist be searchable
namespace std
{
	template <>
	struct hash<QHostAddress>
	{
		size_t operator()(const QHostAddress& v) const
		{
			return std::hash<std::string>()(v.toString().toStdString());
		}
	};
}

message sendBlock(QString str, QString)
{
	if (str.startsWith("/block "))
		return block_message::create(block_message_subtype::ADD,QHostAddress(str.section(' ',1)));
}

message sendUnblock(QString str, QString)
{
	if (str.startsWith("/unblock "))
		return block_message::create(block_message_subtype::REMOVE,QHostAddress(str.section(' ',1)));
}

QByteArray formatMessage(QString type, QString channel, QString nick, QString contents);

void run(plugin_pipe p, string name)
{
	unordered_set<QHostAddress> blocklist;

	//register for the message types the antenna can handle
	p.write(registration_message::create(0, name, "block"));
	p.write(registration_message::create(100, name, "edict"));
	p.write(registration_message::create(100, name, "me_edict"));
	p.write(registration_message::create(0, name, "handler_ready"));

	p.write(register_handler::create("/block", sendBlock));
	p.write(register_handler::create("/unblock", sendUnblock));

	//connect to multicast group
	QUdpSocket udpSocket;
	QHostAddress groupAddress(LIRCH_DEFAULT_ADDR);
	quint16 port = LIRCH_DEFAULT_PORT;

	if (!udpSocket.bind(groupAddress,port))
	{
		cout <<"failed to bind"<<endl;
		return;
	}
	if(!udpSocket.joinMulticastGroup(groupAddress))
	{
		cout<<"failed to join multicast"<<endl;
		return;
	}
	//needed to send nick with your messages
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, "Lirch");
	settings.beginGroup("UserData");

	while(true)
	{
		while (p.has_message())
		{
			message m = p.read();

			if (m.type=="shutdown")
			{
				udpSocket.leaveMulticastGroup(groupAddress);
				udpSocket.close();
				return;
			}
			else if (m.type=="registration_status")
			{
				auto s=dynamic_cast<registration_status *>(m.getdata());
				if (!s)
					continue;
				//Retry 1900 or 2000 times until we succeed
				if (!s->status)
				{
					if (s->priority<2000)
						p.write(registration_message::create(s->priority+1, name, s->type));
				}
			}
			else if(m.type=="handler_ready")
			{
				p.write(register_handler::create("/block", sendBlock));
				p.write(register_handler::create("/unblock", sendUnblock));
				p.write(m.decrement_priority());
			}
			else if(m.type=="block")
			{
				auto castMessage=dynamic_cast<block_message *>(m.getdata());

				//if it's not actually a block message, ignore it and move on
				if (!castMessage)
					continue;

				//this contains the IP that /block or /unblock was called on
				auto toModify=castMessage->ip;

				if(castMessage->subtype==block_message_subtype::ADD)
					blocklist.insert(toModify);
				if(castMessage->subtype==block_message_subtype::REMOVE)
					blocklist.erase(toModify);
			}			
			else if(m.type=="edict")
			{
				auto castMessage=dynamic_cast<edict_message *>(m.getdata());

				//if it's not actually an edict message, ignore it and move on
				if (!castMessage)
					continue;


				QString nick=settings.value("nick","spartacus").value<QString>();
				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QString type;
				if(castMessage->subtype==edict_message_subtype::NORMAL)
					type="edct";
				else if(castMessage->subtype==edict_message_subtype::ME)
					type="mdct";
				QByteArray message = formatMessage(type,channel,nick,contents);

				//change to use write() function when we have time
				if(message.length()>0)
					udpSocket.writeDatagram(message,groupAddress,port);
			}
			else if(m.type=="sendable_notify")
			{
				auto castMessage=dynamic_cast<sendable_notify_message *>(m.getdata());

				//if it's not actually an edict message, ignore it and move on
				if (!castMessage)
					continue;

				QString nick=settings.value("nick","spartacus").value<QString>();
				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QString type="ntfy";

				QByteArray message = formatMessage(type,channel,nick,contents);

				//change to use write() function when we have time
				if(message.length()>0)
					udpSocket.writeDatagram(message,groupAddress,port);
			}
			//if somehow a message is recieved that is not of these types, send it back.
			else
			{
				p.write(m.decrement_priority());
			}
		}

		while (udpSocket.hasPendingDatagrams())
		{
			char broadcast[512];
			QHostAddress senderIP;
			quint16 senderPort;
			qint64 size = udpSocket.readDatagram(broadcast,512,&senderIP,&senderPort);
			broadcast[size]='\0';
			if(blocklist.end()!=blocklist.find(senderIP))
				continue;

			QString destinationChannel=QString::fromUtf8(broadcast+4);
			QString senderNick=QString::fromUtf8(broadcast+68);
			QString sentContents=QString::fromUtf8(broadcast+132);

			string type(broadcast,4);
			if (type=="edct")
			{
				p.write(received_message::create(received_message_subtype::NORMAL,destinationChannel,senderNick,sentContents,senderIP));
			}
			else if (type=="mdct")
			{
				p.write(received_message::create(received_message_subtype::ME,destinationChannel,senderNick,sentContents,senderIP));
			}
			else if (type=="ntfy")
			{
				p.write(received_message::create(received_message_subtype::NOTIFY,destinationChannel,senderNick,sentContents,senderIP));
			}
			else
			{
				continue;
			}

		}

		this_thread::sleep_for(chrono::milliseconds(50));


	}



};

//if components are too long, the cropped version might not have a \0 to terminate it.  might need fixing later.
QByteArray formatMessage(QString type, QString channel, QString nick, QString contents)
{
	QByteArray output;
	output += type.toUtf8();
	output += channel.toUtf8().leftJustified(64,'\0',true);
	output += nick.toUtf8().leftJustified(64,'\0',true);
	QByteArray holder =contents.toUtf8();
	if (holder.length()>256)
	{
		return QByteArray();
	}
	output += holder;
	output += '\0';
	return output;
};
