#ifndef PARSER_H_
#define PARSER_H_

#include <QStringList>

static QStringList parse(QString q)
{
	enum {normal, quote, double_quote, whitespace, escape, special} mode=normal, fallback_mode;
	uint specialchar=0;
	int speciallen=0;
	QStringList strings;
	QString str;
	for (int i=0; i<q.size(); ++i)
	{
		if (mode==normal)
		{
			if (q[i].isSpace())
			{
				strings.append(str);
				str.clear();
				mode=whitespace;
			}
			else if (q[i]=='\"')
			{
				mode=double_quote;
			}
			else if (q[i]=='\'')
			{
				mode=quote;
			}
			else if (q[i]=='\\')
			{
				mode=escape;
				fallback_mode=normal;
			}
			else
				str.append(q[i]);
		}
		else if (mode==quote)
		{
			if (q[i]=='\'')
			{
				mode=normal;
			}
			else
				str.append(q[i]);
		}
		else if (mode==double_quote)
		{
			if (q[i]=='\\')
			{
				mode=escape;
				fallback_mode=double_quote;
			}
			else if (q[i]=='\"')
			{
				mode=normal;
			}
			else
				str.append(q[i]);
		}
		else if (mode==whitespace)
		{
			if (q[i].isSpace())
			{
				//Do nothing so we can skip all the spaces
			}
			else
			{
				//Reparse the non-space character in normal mode
				mode=normal;
				--i;
			}
		}
		else if (mode==escape)
		{
			if (q[i]=='n')
			{
				str.append('\n');
			}
			else if (q[i]=='t')
			{
				str.append('\t');
			}
			else if (q[i]=='x')
			{
				mode=special;
				speciallen=2;
			}
			else if (q[i]=='u')
			{
				mode=special;
				speciallen=4;
			}
			else if (q[i]=='U')
			{
				mode=special;
				speciallen=6;
			}
			else
				str.append(q[i]);
			if (mode==escape)
				mode=fallback_mode;
		}
		else if (mode==special)
		{
			specialchar*=16;
			if ('0'<=q[i] && q[i]<='9')
				specialchar+=q[i].unicode()-'0';
			else if ('a'<=q[i] && q[i]<='f')
				specialchar+=q[i].unicode()-'a'+10;
			else if ('A'<=q[i] && q[i]<='F')
				specialchar+=q[i].unicode()-'A'+10;
			else
			{
				specialchar/=16;
				speciallen=1;
				--i;
			}
			--speciallen;
			if (speciallen==0)
			{
				str.append(QString::fromUcs4(&specialchar, 1));
				specialchar=0;
				mode=fallback_mode;
			}
		}
	}
	if (!str.isEmpty())
		strings.append(str);
	return strings;
}

#endif
