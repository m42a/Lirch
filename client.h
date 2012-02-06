#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>

namespace Ui {
    class Client;
}

class Client : public QMainWindow {
    Q_OBJECT
public:
    Client(QWidget *parent = 0);
    ~Client();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Client *ui;
};

#endif // CLIENT_H
