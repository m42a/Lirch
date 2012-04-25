#include "lirch_setup_wizard.h"
#include "ui_lirch_setup_wizard.h"

LirchSetupWizard::LirchSetupWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::LirchSetupWizard)
{
    ui->setupUi(this);
    // Configure fields (nick is mandatory)
/*
    registerField("UserSettings.nick*", ui->nickname_lineedit);
    registerField("UserSettings.nick_is_default", ui->nickname_checkbox);
    registerField("Logger.log_default", ui->logging_type_default);
    registerField("Logger.log_everything", ui->logging_type_on);
    registerField("Logger.log_nothing", ui->logging_type_off);
    registerField("Logger.root_directory", ui->logging_dir_lineedit);
*/
    // TODO validate file path exists ^
}

LirchSetupWizard::~LirchSetupWizard()
{
    delete ui;
}

QString LirchSetupWizard::get_nick() const {
	return field("UserSettings.nick").value<QString>();
}

bool LirchSetupWizard::nick_is_default() const {
	return field("UserSettings.nick_is_default").value<bool>();
}

logging_message::logging_mode LirchSetupWizard::get_logging_mode() const {
	if (field("Logger.log_default").value<bool>()) {
		return logging_message::logging_mode::DEFAULT;
	}
	if (field("Logger.log_everything").value<bool>()) {
		return logging_message::logging_mode::ON;
	}
	if (field("Logger.log_nothing").value<bool>()) {
		return logging_message::logging_mode::OFF;
	}
	return logging_message::logging_mode::DEFAULT;
}

QString LirchSetupWizard::get_logging_directory() const {
	return field("Logger.root_directory").value<QString>();
}

void LirchSetupWizard::changeEvent(QEvent *e)
{
    QWizard::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
