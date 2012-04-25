#ifndef EDITOR_H_
#define EDITOR_H_

#include <QString>

// Inspired by http://cnswww.cns.cwru.edu/php/chet/readline/readline.html#SEC13

//Right now we only support the cursor being at the end of the line, but
//ideally that will change.
class text_line
{
public:
	QString getQString() {return line;}
	int getPosition() {return line.length();}

	void insert(wchar_t c)
	{
		line.push_back(QString::fromWCharArray(&c, 1));
	}

	void delete_char()
	{
	}

	void backward_delete_char()
	{
		QTextBoundaryFinder bounds(QTextBoundaryFinder::Grapheme, line);
		bounds.toEnd();
		int pos=bounds.toPreviousBoundary();
		if (pos!=-1)
		{
			line.remove(pos, INT_MAX);
		}
	}

	void kill_line()
	{
	}

	void backward_kill_line()
	{
		line="";
	}

	void kill_whole_line()
	{
		line="";
	}
private:
	QString line;
};

#endif
