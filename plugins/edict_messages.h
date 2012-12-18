/*
 * raw_edict and edict were kept separate because modules are unlikely to take in both types.
 * they contain the destination channel and the contents of the message.
 * in the case of edict, it has a subtype of Normal or Me, in order for them to be used in the appropriate manner
 */

#ifndef EDICT_MESSAGES_H_
#define EDICT_MESSAGES_H_

#include <QString>

#include "core/message.h"

enum class edict_message_subtype
{
	NORMAL,ME
};

struct raw_edict_message
{
	static constexpr auto message_id="raw_edict";

	QString contents;
	QString channel;
};

struct edict_message
{
	static constexpr auto message_id="edict";

	edict_message_subtype subtype;
	QString channel;
	QString contents;
};

#endif
