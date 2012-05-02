#ifndef LIRCH_SETUPWIZARD_H
#define LIRCH_SETUPWIZARD_H

#include <QEvent>
#include <QObject>
#include <QString>
#include <QWidget>
#include <QWizard>

#include "plugins/logger_messages.h"

namespace Ui {
    class LirchSetupWizard;
}

class LirchSetupWizard : public QWizard {
    Q_OBJECT
public:
    LirchSetupWizard(QWidget *parent = 0);
    ~LirchSetupWizard();

    // Field Accessors
    QString get_nick() const;
    bool nick_is_default() const;
    logging_message::logging_mode get_logging_mode() const;
    QString get_logging_directory() const;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::LirchSetupWizard *ui;
};

#endif // LIRCH_SETUPWIZARD_H
