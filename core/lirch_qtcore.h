#include <mutex>
#include <thread>

#include <QObject>

#include "core/message.h"
#include "core/message_pipe.h"
#include "core/core_messages.h"

class core_mediator : public QObject {
	Q_OBJECT
public:
	core_mediator(std::thread *t = nullptr) :
		core_thread(t) { }

	void load(std::thread *t) {
		if (thread_mutex.try_lock()) {
			if (!core_thread) {
				core_thread = t;
			}
			thread_mutex.unlock();
		}
	}

public slots:
	void shutdown() {
		if (thread_mutex.try_lock()) {
			if (core_thread && core_thread->joinable()) {
				extern message_pipe in_pipe;
				in_pipe.write(core_quit_message::create());
				core_thread->join();
				core_thread = nullptr;
			}
			thread_mutex.unlock();
			emit aboutToShutdown();
		}
	}

signals:
	void aboutToShutdown();

private:
	std::thread *core_thread;
	std::mutex thread_mutex;
};
