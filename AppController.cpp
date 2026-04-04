#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

QString AppController::appTitle() const
{
    return QStringLiteral("FourPillarsOfDestiny");
}

QString AppController::helloMessage() const
{
    return QStringLiteral("泰山流四柱推命ソフトの初期画面です。");
}
