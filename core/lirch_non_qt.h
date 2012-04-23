#include <thread>
#include <vector>

#include <QObject>

//Yes, these have to go in a header file.  No, I don't know why.
class core_waiter : public QObject
{
	Q_OBJECT

public:
	core_waiter(std::thread &tt) : t(tt) {}

public slots:
	void onQuit() {t.join();}

private:
	std::thread &t;
};

class core_notifier : public QObject
{
	Q_OBJECT

public:
	void emitQuit() {emit quit();}

signals:
	void quit();
};
