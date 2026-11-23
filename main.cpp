#include "AppLanguage.h"
#include "keypresserHardware.h"

#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include <QUrl>

namespace {
void showProfessionalEditionPrompt(QWidget *parent)
{
    QSettings settings(QStringLiteral("FinnSoft"), QStringLiteral("KeyPresserHardware"));
    settings.beginGroup(QStringLiteral("promotion"));
    if (settings.value(QStringLiteral("dismissed"), false).toBool())
        return;

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QDateTime lastShown = settings.value(QStringLiteral("lastShown")).toDateTime();
    if (lastShown.isValid() && now < lastShown.addDays(7))
        return;

    auto *prompt = new QMessageBox(
        QMessageBox::Information,
        QCoreApplication::translate("StartupPromotion", "试试 KeyPresser 专业版"),
        QCoreApplication::translate("StartupPromotion", "KeyPresser 专业版支持窗口录制回放、图像识别、鼠标自动化和 Python 脚本。"),
        QMessageBox::NoButton, parent);
    prompt->setAttribute(Qt::WA_DeleteOnClose);
    auto *learnMore = prompt->addButton(QCoreApplication::translate("StartupPromotion", "了解专业版"),
                                        QMessageBox::AcceptRole);
    auto *later = prompt->addButton(QCoreApplication::translate("StartupPromotion", "稍后再说"),
                                    QMessageBox::RejectRole);
    prompt->setDefaultButton(later);
    prompt->setEscapeButton(later);
    auto *dismiss = new QCheckBox(QCoreApplication::translate("StartupPromotion", "不再显示"), prompt);
    prompt->setCheckBox(dismiss);
    QObject::connect(prompt, &QMessageBox::finished, parent, [prompt, dismiss, learnMore]() {
        if (dismiss->isChecked()) {
            QSettings preferences(QStringLiteral("FinnSoft"), QStringLiteral("KeyPresserHardware"));
            preferences.setValue(QStringLiteral("promotion/dismissed"), true);
        }
        if (prompt->clickedButton() == learnMore) {
            QDesktopServices::openUrl(QUrl(AppLanguage::isEnglish()
                ? QStringLiteral("https://keypresser.onepeak.cn/index-en.html#download")
                : QStringLiteral("https://keypresser.onepeak.cn/#download")));
        }
    });

    settings.setValue(QStringLiteral("lastShown"), now);
    prompt->open();
}
}

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
        QTimer::singleShot(0, &keyPresser, [&keyPresser]() { showProfessionalEditionPrompt(&keyPresser); });
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
