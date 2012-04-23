/*
 * IP address for the multicast group 224.0.0.224
 * Port for multicast group 45454
 *
 *
 * broadcasts are of the format
 * [type][channel][nick][contents]
 * type is a 4 byte string, currently "edct" for normal edicts, "mdct" for medicts, "ntfy" for notifies, "whhe" for who is here, "here" for here, "nick" for nick changes.
 * channel is a 64 byte string which contains the destination channel for the message terminated with zero characters.
 * nick is the same size and idea as channel, except it contains the nick of the sender.
 * contents is a max 256 byte string of whatever the text being sent is.  If the contents are shorter, the broadcast is shorter to match.
 *
 * in the case of a change of nick, the old nickname is in the channel slot, since nick changes are channelless
 * and nicks and channels take the same amount of space.
 *
 * a periodic broadcast of type "auto" is sent out every minute; it's format is
 * [type][nick]
 * type is the standard 4 byte string, and nick is a max 64 byte
 *
 * To Do:
 * make sure the nick/channel/message length being broadcast is short enough
 * toss a notify to the UI if ^ fails
 *
 * add the timed "still here" messages
 */


#include <thread>
#include <iostream>
#include <QByteArray>
#include <QString>
#include <QtNetwork>
#include <unordered_set>
#include <ctime>

#include "lirch_constants.h"
#include "blocker_messages.h"
#include "edict_messages.h"
#include "received_messages.h"
#include "lirch_plugin.h"
#include "lirch_constants.h"
#include "grinder_messages.h"
#include "notify_messages.h"
#include "nick_messages.h"
#include "userlist_messages.h"

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
	return empty_message::create();
}

message sendUnblock(QString str, QString)
{
	if (str.startsWith("/unblock "))
		return block_message::create(block_message_subtype::REMOVE,QHostAddress(str.section(' ',1)));
	return empty_message::create();
}

QByteArray formatMessage(QString type, QString channel, QString nick, QString contents);

void run(plugin_pipe p, string name)
{
	unordered_set<QHostAddress> blocklist;

	time_t lastSent=time(NULL);

	//register for the message types the antenna can handle
	p.write(registration_message::create(0, name, "block"));
	p.write(registration_message::create(0, name, "edict"));
	p.write(registration_message::create(0, name, "who is here"));
	p.write(registration_message::create(0, name, "handler_ready"));
	p.write(registration_message::create(0, name, "changed_nick"));

	p.write(register_handler::create("/block", sendBlock));
	p.write(register_handler::create("/unblock", sendUnblock));

	//connect to multicast group
	QUdpSocket udpSocket;
	QHostAddress groupAddress(LIRCH_DEFAULT_ADDR);
	quint16 port = LIRCH_DEFAULT_PORT;


	//TODO: Explicitly set QAbstractSocket::MulticastLoopbackOption to 1
	if (!udpSocket.bind(groupAddress,port,QUdpSocket::ShareAddress))
	{
		p.write(notify_message::create("default","Failed to bind."));
		return;
	}
	if(!udpSocket.joinMulticastGroup(groupAddress))
	{
		p.write(notify_message::create("default","Failed to join Multicast Group."));
		return;
	}

	QString currentNick = LIRCH_DEFAULT_NICK;

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
				//Retry 2000 times until we succeed
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
				{
					blocklist.insert(toModify);
					p.write(notify_message::create("default",toModify.toString()+" is now blocked."));
				}
				if(castMessage->subtype==block_message_subtype::REMOVE)
				{
					blocklist.erase(toModify);
					p.write(notify_message::create("default",toModify.toString()+" is now unblocked."));
				}
			}			
			else if(m.type=="edict")
			{
				auto castMessage=dynamic_cast<edict_message *>(m.getdata());

				//if it's not actually an edict message, ignore it and move on
				if (!castMessage)
					continue;


				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QString type;
				if(castMessage->subtype==edict_message_subtype::NORMAL)
					type="edct";
				else if(castMessage->subtype==edict_message_subtype::ME)
					type="mdct";
				QByteArray message = formatMessage(type,channel,currentNick,contents);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
				else
					p.write(notify_message::create(channel,"Message too long."));
			}
			else if(m.type=="sendable_notify")
			{
				auto castMessage=dynamic_cast<sendable_notify_message *>(m.getdata());

				//if it's not actually a notify message, ignore it and move on
				if (!castMessage)
					continue;

				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QString type="ntfy";

				QByteArray message = formatMessage(type,channel,currentNick,contents);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
				else
					p.write(notify_message::create(channel,"Notify message too long. No idea how you did that."));
			}
			else if(m.type=="who is here")
			{
				auto castMessage=dynamic_cast<who_is_here_message *>(m.getdata());

				//if it's not actually a who's here message, ignore it and move on
				if (!castMessage)
					continue;

				QString channel=castMessage->channel;
				QString type="whhe";

				QByteArray message = formatMessage(type,channel,currentNick,"");

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
			}
			else if(m.type=="here")
			{
				auto castMessage=dynamic_cast<who_is_here_message *>(m.getdata());

				//if it's not actually a here message, ignore it and move on
				if (!castMessage)
					continue;

				QString channel=castMessage->channel;
				QString type="here";

				QByteArray message = formatMessage(type,channel,currentNick,"");

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
			}
			else if (m.type == "changed_nick")
			{
				auto castMessage=dynamic_cast<changed_nick_message *>(m.getdata());

				//if it's not actually a here message, ignore it and move on
				if (!castMessage)
					continue;


				QString type = "nick";

				QByteArray message = formatMessage(type,currentNick,castMessage->newNick,"");

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}

				currentNick=castMessage->newNick;
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

			string type(broadcast,4);

			if (type=="auto")
			{
				p.write(received_message::create(received_message_subtype::HERE,"",QString::fromUtf8(broadcast+4),"",senderIP));
				continue;
			}

			//takes the components out of the broadcast and crops them apropriately, just in case
			QString destinationChannel=QString::fromUtf8(broadcast+4,64).section(QChar('\0'),0,0);
			QString senderNick=QString::fromUtf8(broadcast+68,64).section(QChar('\0'),0,0);

			if (type=="whhe")
			{
				p.write(received_message::create(received_message_subtype::WHOHERE,destinationChannel,senderNick,"",senderIP));
				continue;
			}
			else if (type=="here")
			{
				p.write(received_message::create(received_message_subtype::HERE,destinationChannel,senderNick,"",senderIP));
				continue;
			}
			else if (type=="nick")
			{
				p.write(received_message::create(received_message_subtype::NICK,destinationChannel,senderNick,"",senderIP));
				continue;
			}

			QString sentContents=QString::fromUtf8(broadcast+132,256).section(QChar('\0'),0,0);

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


		//sends out a "still here" message every minute
		if(time(NULL)-lastSent>60)
		{
			QString type="auto";

			QByteArray message = type.toUtf8()+currentNick.toUtf8();
			message.truncate(68);

			udpSocket.writeDatagram(message,groupAddress,port);
			lastSent=time(NULL);
		}
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}

//stores type in he first 4 bytes, channel in the 64 after that, then 64 for nick, and up to 256 for the contents of the message
QByteArray formatMessage(QString type, QString channel, QString nick, QString contents)
{
	QByteArray output;
	output += type.toUtf8();
	output += channel.toUtf8().leftJustified(64,'\0',true);
	output += nick.toUtf8().leftJustified(64,'\0',true);
	QByteArray holder =contents.toUtf8();

	if (holder.length()>256)
		return QByteArray();

	output += holder;
	output += '\0';
	return output;
}
