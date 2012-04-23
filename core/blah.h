#include <thread>

#include <QObject>

class dummy : public QObject
{
	Q_OBJECT

public:
	dummy(std::thread *tt) : t(tt) {}

public slots:
	void willQuit() {t->join();}

private:
	std::thread *t;
};

class dummy2 : public QObject
{
	Q_OBJECT

public:
	void quit() {emit willQuit();}

signals:
	void willQuit();
};

