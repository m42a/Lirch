/*
 * broadcasts are of the format
 * [type][channel][nick][contents]
 * type is a 4 byte string, currently "edct" for normal edicts, "mdct" for medicts.
 * channel is a 64 byte string which contains the destination channel for the message terminated with zero characters.
 * nick is the same size and idea as channel, except it contains the nick of the sender.
 * contents is a max 256 byte string of whatever the text being sent is.  If the contents are shorter, the broadcast is shorter to match.
 */


#include <thread>
#include <iostream>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtNetwork>
#include <unordered_set>

#include "blocker_messages.h"
#include "edict_messages.h"
#include "received_messages.h"
#include "lirch_plugin.h"

using namespace std;

namespace std
{

	template <>
	struct hash<QHostAddress>
	{size_t operator()(const QHostAddress& v) const
		{
			return std::hash<std::string>()(v.toString().toStdString());
		}
	};

}


QByteArray formatMessage(QString type, QString channel, QString nick, QString contents);

void run(plugin_pipe p, string name)
{
	unordered_set<QHostAddress> blocklist;

	//register for the message types the antenna can handle
	p.write(registration_message::create(0, name, "block"));
	p.write(registration_message::create(0, name, "unblock"));
	p.write(registration_message::create(100, name, "edict"));
	p.write(registration_message::create(100, name, "me_edict"));



	//connect to multicast group
	QUdpSocket udpSocket;
	QHostAddress groupAddress("224.0.0.224");
	quint16 port = 45454;

	if (!(udpSocket.bind(groupAddress,port) && udpSocket.joinMulticastGroup(groupAddress)))
	{
		//flip out, you failed to connect
	}

	while(true)
	{
		while (p.has_message())
		{
			message m = p.read();

			if (m.type=="shutdown")
			{
				//disconnect from multicast group
				return;
			}
			else if (m.type=="registration_status")
			{
				auto s=dynamic_cast<registration_status *>(m.getdata());
				if (!s)
					continue;
				//Retry 2000 times until we succeed
				if (!s->status && s->priority>30000)
					p.write(registration_message::create(s->priority-1, name, s->type));
			}
			else if(m.type=="block")
			{
				auto castMessage=dynamic_cast<block_message *>(m.getdata());

				//if it's not actually a block message, ignore it and move on
				if (!castMessage)
					continue;

				auto toBlock=castMessage->ip;

				//adds the ip from m to the blocklist
				//if it is already there, it does nothing.
				blocklist.insert(toBlock);
			}
			else if(m.type=="unblock")
			{
				auto castMessage=dynamic_cast<unblock_message *>(m.getdata());

				//if it's not actually an unblock message, ignore it and move on
				if (!castMessage)
					continue;

				auto toUnblock=castMessage->ip;

				//removes the ip in castMessage from the blocklist
				//if it is not there, it does nothing.
				blocklist.erase(toUnblock);
			}
			else if(m.type=="edict")
			{
				auto castMessage=dynamic_cast<edict_message *>(m.getdata());

				//if it's not actually an edict message, ignore it and move on
				if (!castMessage)
					continue;


				QString nick="";
				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QByteArray message = formatMessage("edct",channel,nick,contents);

				udpSocket.writeDatagram(message,groupAddress,port);
			}
			else if(m.type=="me_edict")
			{
				auto castMessage=dynamic_cast<me_edict_message *>(m.getdata());

				//if it's not actually a medict message, ignore it and move on
				if (!castMessage)
					continue;

				QString nick="";
				QString channel=castMessage->channel;
				QString contents=castMessage->contents;
				QByteArray message = formatMessage("mdct",channel,nick,contents);

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
			udpSocket.readDatagram(broadcast,512,senderIP,45454);

			QString destinationChannel=QString::toUtf8(broadcast+4);
			QString senderNick=QString::toUtf8(broadcast+68);
			QString sentContents=QString::toUtf8(broadcast+132);

			string type(broadcast,4);
			if (type=="edct")
			{
				p.write(received_message::create(destinationChannel,SenderNick,sentContents));
			}
			else if (type=="mdct")
			{
				p.write(received_me_message::create(destinationChannel,SenderNick,sentContents));
			}
			else
			{
				continue;
			}

		}


	}



};

//if components are too long, the cropped version might not have a \0 to terminate it.  might need fixing later.
QByteArray formatMessage(QString type, QString channel, QString nick, QString contents)
{
	QByteArray output;
	output += type.toUtf8();
	output += channel.toUtf8().leftJustified(64,'\0',true);
	output += nick.toUtf8().leftJustified(64,'\0',true);
	output += contents.toUtf8().truncate(256);
	return output;
};
