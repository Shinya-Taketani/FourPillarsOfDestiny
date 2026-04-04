#pragma once

#include <QObject>
#include <QString>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appTitle READ appTitle CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);

    QString appTitle() const;

    Q_INVOKABLE QString helloMessage() const;
};
