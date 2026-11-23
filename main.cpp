#include "AppLanguage.h"
#include "keypresserHardware.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    AppLanguage::initialize();

    QFile styleFile(QStringLiteral(":/style.qss"));
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
    }

    int result;
    {
        KeyPresserHardware keyPresser;
        keyPresser.checkArduino();
        keyPresser.show();
        result = app.exec();
    }

    if (result != AppLanguage::restartExitCode)
        return result;
    if (QProcess::startDetached(app.applicationFilePath(), {}))
        return 0;

    QMessageBox::warning(nullptr, QStringLiteral("KeyPresser Hardware"),
                         AppLanguage::isEnglish()
                             ? QStringLiteral("Please reopen KeyPresser Hardware to apply the language change.")
                             : QStringLiteral("请重新打开 KeyPresser 硬件版以应用语言设置。"));
    return 1;
}
