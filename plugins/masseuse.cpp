/*
 * this module converts messages into a format able to be displayed by the UI.
 *
 * it can currently convert messages and me messages that the user sends, as well
 * as the same types of messages received by the antenna.
 * it then converts them into a display_message, which will be used by the UI and Logger.
 */



#include <QtCore>
#include <QSettings>

#include "edict_messages.h"
#include "received_messages.h"
#include "lirch_plugin.h"
#include "lirch_constants.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	//register for the message types the antenna can handle
	p.write(registration_message::create(-30000, name, "received"));
	p.write(registration_message::create(-30000, name, "received_me"));
	p.write(registration_message::create(-30000, name, "edict"));
	p.write(registration_message::create(-30000, name, "me_edict"));

	//needed to send nick with your messages
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, "Lirch");
	settings.beginGroup("UserData");

	while(true)
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
			//Retry 2000 times until we succeed
			if (!s->status && s->priority<-28000)
				p.write(registration_message::create(s->priority+1, name, s->type));
		}
		else if (m.type=="edict")
		{
			auto castMessage=dynamic_cast<edict_message *>(m.getdata());

			//if it's not actually an edict message, ignore it and move on
			if (!castMessage)
				continue;

			QString nick=settings.value("nick","spartacus").value<QString>();
			p.write(display_message::create(castMessage->channel,castMessage->contents,nick));
		}
		else if (m.type=="me_edict")
		{
			auto castMessage=dynamic_cast<me_edict_message *>(m.getdata());

			//if it's not actually a medict message, ignore it and move on
			if (!castMessage)
				continue;

			QString nick=settings.value("nick","spartacus").value<QString>();
			p.write(display_message::create(castMessage->channel,castMessage->contents,nick));
		}
		else if (m.type=="received")
		{
			auto castMessage=dynamic_cast<received_message *>(m.getdata());

			//if it's not actually a received message, ignore it and move on
			if (!castMessage)
				continue;

			p.write(display_message::create(castMessage->channel,castMessage->contents,castMessage->nick));
		}
		else if (m.type=="received_me")
		{
			auto castMessage=dynamic_cast<received_me_message *>(m.getdata());

			//if it's not actually a received me message, ignore it and move on
			if (!castMessage)
				continue;

			p.write(display_message::create(castMessage->channel,castMessage->contents,castMessage->nick));
		}
	}
}
