#include <thread>
#include <vector>

#include <QObject>

#include "core/message.h"
#include "core/message_pipe.h"
#include "core/core_messages.h"

class core_mediator : public QObject {
	Q_OBJECT
	typedef void (*core_function)(const std::vector<message> &);
public:
	core_mediator(core_function func, const std::vector<message> &msgs) :
		core_thread(new std::thread(func, msgs)) { }
	~core_mediator() { shutdown(); }

public slots:
	void shutdown() {
		if (shutdown_mutex.try_lock()) {
			extern message_pipe in_pipe;
			in_pipe.write(core_quit_message::create());
			if (core_thread) {
				core_thread->join();
				delete core_thread;
				core_thread = nullptr;
			}
			shutdown_mutex.unlock();
			emit aboutToShutdown();
		}
	}

signals:
	void aboutToShutdown();

private:
	std::thread *core_thread;
	std::mutex shutdown_mutex;
};
