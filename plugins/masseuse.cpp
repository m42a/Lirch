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
#include "display_messages.h"
#include "received_messages.h"
#include "notify_messages.h"
#include "lirch_plugin.h"
#include "lirch_constants.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	//register for the message types the display adapter can handle
	p.write(registration_message::create(-30000, name, "received"));
	p.write(registration_message::create(-30000, name, "notify"));

	//needed to send nick with your messages
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, "Lirch");
	settings.beginGroup("UserData");

	while(true)
	{
		//waits until any messages are sent to it
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

		//message parsing is simple, just putting the right things in the right fields.
		else if (m.type=="received")
		{
			auto castMessage=dynamic_cast<received_message *>(m.getdata());

			//if it's not actually a received message, ignore it and move on
			if (!castMessage)
				continue;

			if(castMessage->subtype==received_message_subtype::NORMAL)
				p.write(display_message::create(display_message_subtype::NORMAL,castMessage->channel,castMessage->nick,castMessage->contents));
			else if(castMessage->subtype==received_message_subtype::ME)
				p.write(display_message::create(display_message_subtype::ME,castMessage->channel,castMessage->nick,castMessage->contents));
			else if(castMessage->subtype==received_message_subtype::NOTIFY)
				p.write(display_message::create(display_message_subtype::NOTIFY,castMessage->channel,castMessage->nick,castMessage->contents));
			p.write(m.decrement_priority());
		}
		else if (m.type=="notify")
		{
			auto castMessage=dynamic_cast<notify_message *>(m.getdata());

			//if it's not actually a notify me message, ignore it and move on
			if (!castMessage)
				continue;

			p.write(display_message::create(display_message_subtype::NOTIFY,castMessage->channel,"",castMessage->contents));
		}


		//what is this doing here? take it back, i don't want it.
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
