#include "AppLanguage.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

namespace {
bool activeEnglish = false;
}

void AppLanguage::initialize()
{
    QSettings preferences(QStringLiteral("FinnSoft"), QStringLiteral("KeyPresserHardware"));
    const bool defaultEnglish = QLocale::system().language() != QLocale::Chinese;
    activeEnglish = preferences.value(QStringLiteral("uiLanguage"),
                                      defaultEnglish ? QStringLiteral("en") : QStringLiteral("zh-CN"))
                        .toString() == QStringLiteral("en");

    auto *translator = new QTranslator(QCoreApplication::instance());
    const bool loaded = activeEnglish
        ? translator->load(QStringLiteral(":/i18n/KeyPresserHardware_en.qm"))
        : (translator->load(QStringLiteral("qt_zh_CN"),
                           QCoreApplication::applicationDirPath() + QStringLiteral("/translations")) ||
           translator->load(QStringLiteral("qt_zh_CN"), QLibraryInfo::location(QLibraryInfo::TranslationsPath)));
    if (loaded)
        QCoreApplication::installTranslator(translator);
}

bool AppLanguage::isEnglish()
{
    return activeEnglish;
}

void AppLanguage::setPreferredEnglish(bool english)
{
    QSettings preferences(QStringLiteral("FinnSoft"), QStringLiteral("KeyPresserHardware"));
    preferences.setValue(QStringLiteral("uiLanguage"), english ? QStringLiteral("en") : QStringLiteral("zh-CN"));
}
