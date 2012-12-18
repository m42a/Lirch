/*
 * IP address for the multicast group 224.0.0.224
 * Port for multicast group 45454
 *
 *
 * broadcasts are of the format
 * [type][channel][nick][contents]
 * type is a 4 byte string, currently "edct" for normal edicts, "mdct" for medicts, "ntfy" for notifies, "whhe" for who is here, "here" for here, "nick" for nick changes, "left" for announcing you left a channel, "join" for announcing you joined a channel.
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
#include "QHostAddress_hash.h"
#include "nick_messages.h"
#include "userlist_messages.h"
#include "channel_messages.h"

using namespace std;

message sendBlock(QString str, QString channel)
{
	if (str.startsWith("/block "))
	{
		if(!QHostAddress(str.section(' ',1)).isNull())
			return message::create<block_message>(block_message_subtype::ADD,QHostAddress(str.section(' ',1)));
		return message::create<block_name_message>(str.section(' ',1), channel,block_name_message_subtype::ADD);
	}
	if (str.startsWith("/block"))
		return message::create<display_blocks_message>(channel);
	return message::create<empty_message>();
}

message sendUnblock(QString str, QString channel)
{
	if (str.startsWith("/unblock "))
	{
		if(!QHostAddress(str.section(' ',1)).isNull())
			return message::create<block_message>(block_message_subtype::REMOVE,QHostAddress(str.section(' ',1)));
		return message::create<block_name_message>(str.section(' ',1), channel,block_name_message_subtype::REMOVE);
	}
	return message::create<empty_message>();
}

QByteArray formatMessage(QString type, QString channel, QString nick, QString contents, int &flag);

void run(plugin_pipe p, string name)
{
	unordered_set<QHostAddress> blocklist;

	time_t lastSent=time(NULL);

	//register for the message types the antenna can handle
	p.write<registration_message>(0, name, "block");
	p.write<registration_message>(0, name, "edict");
	p.write<registration_message>(0, name, "here");
	p.write<registration_message>(0, name, "who is here");
	p.write<registration_message>(0, name, "handler_ready");
	p.write<registration_message>(0, name, "block query");
	p.write<registration_message>(0, name, "changed_nick");
	p.write<registration_message>(0, name, "sendable_notify");
	p.write<registration_message>(0, name, "leave_channel");
	p.write<registration_message>(0, name, "set_channel");
	p.write<registration_message>(0, name, "display blocks");

	//connect to multicast group
	QUdpSocket udpSocket;
	QHostAddress groupAddress(LIRCH_DEFAULT_ADDR);
	quint16 port = LIRCH_DEFAULT_PORT;


	//TODO: Explicitly set QAbstractSocket::MulticastLoopbackOption to 1
	if (!udpSocket.bind(groupAddress,port,QUdpSocket::ShareAddress))
	{
		p.write<notify_message>("default","Failed to bind.");
		return;
	}
	if(!udpSocket.joinMulticastGroup(groupAddress))
	{
		p.write<notify_message>("default","Failed to join Multicast Group.");
		return;
	}

	QString currentNick = LIRCH_DEFAULT_NICK;

	while(true)
	{
		while (p.has_message())
		{
			message m = p.read();

			if (m.is<shutdown_message>())
			{
				QString channel="";
				QString type="left";
				QString contents="";
				int flag = 0;

				QByteArray message = formatMessage(type,channel,currentNick,contents,flag);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}

				udpSocket.leaveMulticastGroup(groupAddress);
				udpSocket.close();
				return;
			}
			else if (auto s=m.try_extract<registration_status>())
			{
				//Retry 2000 times until we succeed
				if (!s->status)
				{
					if (s->priority<2000)
						p.write<registration_message>(s->priority+1, name, s->type);
				}
				else
				{
					if (s->type=="handler_ready")
					{
						p.write<register_handler>("/block", sendBlock);
						p.write<register_handler>("/unblock", sendUnblock);
					}
				}
			}
			else if(m.is<handler_ready>())
			{
				p.write<register_handler>("/block", sendBlock);
				p.write<register_handler>("/unblock", sendUnblock);
				p.write(m.decrement_priority());
			}
			else if(auto castMessage=m.try_extract<block_message>())
			{
				p.write(m.decrement_priority());

				//this contains the IP that /block or /unblock was called on
				auto toModify=castMessage->ip;

				if(castMessage->subtype==block_message_subtype::ADD)
				{
					blocklist.insert(toModify);
					p.write<notify_message>("default",toModify.toString()+" is now blocked.");
				}
				if(castMessage->subtype==block_message_subtype::REMOVE)
				{
					blocklist.erase(toModify);
					p.write<notify_message>("default",toModify.toString()+" is now unblocked.");
				}
				if(castMessage->subtype == block_message_subtype::QUERY)
				{
					p.write<block_status_message>(castMessage->ip, blocklist.count(castMessage->ip)!=0);
				}
			}
			else if(auto castMessage=m.try_extract<display_blocks_message>())
			{
				p.write(m.decrement_priority());

				for(auto iter = blocklist.begin(); iter != blocklist.end(); iter++)
					p.write<notify_message>(castMessage->channel, iter->toString()+" is blocked");
			}
			else if(auto castMessage=m.try_extract<edict_message>())
			{
				p.write(m.decrement_priority());

				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QString type;
				if(castMessage->subtype==edict_message_subtype::NORMAL)
					type="edct";
				else if(castMessage->subtype==edict_message_subtype::ME)
					type="mdct";

				int flag = 0;
				QByteArray message = formatMessage(type,channel,currentNick,contents,flag);

				//change to use write() function when we have time
				switch (flag)
				{
					case 1:
						p.write<notify_message>(channel,"Channel name too long.");
						break;
					case 2:
						p.write<notify_message>(channel,"Nick too long.");
						break;
					case 3:
						p.write<notify_message>(channel,"Message too long.");
						break;
					default:
						udpSocket.writeDatagram(message,groupAddress,port);
						lastSent=time(NULL);
				}
			}
			else if(auto castMessage=m.try_extract<sendable_notify_message>())
			{
				p.write(m.decrement_priority());

				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QString type="ntfy";
				int flag = 0;

				QByteArray message = formatMessage(type,channel,currentNick,contents,flag);

				//change to use write() function when we have time
				switch (flag)
				{
					case 1:
						p.write<notify_message>(channel,"Channel name too long.");
						break;
					case 2:
						p.write<notify_message>(channel,"Nick too long.");
						break;
					case 3:
						p.write<notify_message>(channel,"Message too long.");
						break;
					default:
						udpSocket.writeDatagram(message,groupAddress,port);
						lastSent=time(NULL);
				}
			}
			else if(m.is<block_query_message>())
			{
				p.write(m.decrement_priority());
				p.write<block_list_message>(blocklist);
			}
			else if(auto castMessage=m.try_extract<who_is_here_message>())
			{
				p.write(m.decrement_priority());

				QString channel=castMessage->channel;
				QString type="whhe";
				int flag = 0;

				QByteArray message = formatMessage(type,channel,currentNick,"",flag);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
			}
			else if(auto castMessage=m.try_extract<here_message>())
			{
				p.write(m.decrement_priority());

				QString channel=castMessage->channel;
				QString type="here";
				int flag = 0;

				QByteArray message = formatMessage(type,channel,currentNick,"",flag);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
			}
			else if (auto castMessage=m.try_extract<changed_nick_message>())
			{
				p.write(m.decrement_priority());

				QString type = "nick";
				int flag = 0;

				QByteArray message = formatMessage(type,currentNick,castMessage->newNick,"",flag);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}

				currentNick=castMessage->newNick;
			}
			else if (auto castMessage=m.try_extract<set_channel_message>())
			{
				p.write(m.decrement_priority());

				QString channel = castMessage->channel;
				QString type = "join";
				int flag = 0;

				QByteArray message = formatMessage(type,channel,currentNick,"",flag);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
			}
			else if (auto castMessage=m.try_extract<leave_channel_message>())
			{
				p.write(m.decrement_priority());

				QString channel = castMessage->channel;
				QString type = "left";
				int flag = 0;

				QByteArray message = formatMessage(type,channel,currentNick,"",flag);

				//change to use write() function when we have time
				if(message.length()>0)
				{
					udpSocket.writeDatagram(message,groupAddress,port);
					lastSent=time(NULL);
				}
			}
			//if somehow a message is recieved that is not of these types, send it back.
			else
			{
				p.write(m.decrement_priority());
			}
		}

		if (udpSocket.waitForReadyRead(50))
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
				p.write<received_status_message>(received_status_message_subtype::HERE,"",QString::fromUtf8(broadcast+4),senderIP);
				continue;
			}

			//takes the components out of the broadcast and crops them apropriately, just in case
			QString destinationChannel=QString::fromUtf8(broadcast+4,64).section(QChar('\0'),0,0);
			QString senderNick=QString::fromUtf8(broadcast+68,64).section(QChar('\0'),0,0);

			if (type=="whhe")
			{
				p.write<received_status_message>(received_status_message_subtype::WHOHERE,destinationChannel,senderNick,senderIP);
				continue;
			}
			else if (type=="here")
			{
				p.write<received_status_message>(received_status_message_subtype::HERE,destinationChannel,senderNick,senderIP);
				continue;
			}
			else if (type=="nick")
			{
				p.write<received_status_message>(received_status_message_subtype::NICK,destinationChannel,senderNick,senderIP);
				continue;
			}
			else if (type=="left")
			{
				p.write<received_status_message>(received_status_message_subtype::LEFT,destinationChannel,senderNick,senderIP);
				continue;
			}
			else if (type=="join")
			{
				p.write<received_status_message>(received_status_message_subtype::JOIN,destinationChannel,senderNick,senderIP);
				continue;
			}

			QString sentContents=QString::fromUtf8(broadcast+132,256).section(QChar('\0'),0,0);

			if (type=="edct")
			{
				p.write<received_message>(received_message_subtype::NORMAL,destinationChannel,senderNick,sentContents,senderIP);
			}
			else if (type=="mdct")
			{
				p.write<received_message>(received_message_subtype::ME,destinationChannel,senderNick,sentContents,senderIP);
			}
			else if (type=="ntfy")
			{
				p.write<received_message>(received_message_subtype::NOTIFY,destinationChannel,senderNick,sentContents,senderIP);
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
	}
}

//stores type in he first 4 bytes, channel in the 64 after that, then 64 for nick, and up to 256 for the contents of the message
//flag will be set to a value based on why the messages failes to be formatted
//1 if channel, 2 if nick, 3 if contents
QByteArray formatMessage(QString type, QString channel, QString nick, QString contents, int &flag)
{
	flag = 0;

	if (channel.toUtf8().length()>64)
	{
		flag = 1;
		return QByteArray();
	}
	if (nick.toUtf8().length()>64)
	{
		flag = 2;
		return QByteArray();
	}
	if (contents.toUtf8().length()>256)
	{
		flag = 3;
		return QByteArray();
	}

	QByteArray output;
	output += type.toUtf8();
	output += channel.toUtf8().leftJustified(64,'\0',true);
	output += nick.toUtf8().leftJustified(64,'\0',true);
	output += contents.toUtf8();
	output += '\0';
	return output;
}
