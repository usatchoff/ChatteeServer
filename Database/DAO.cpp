#include <cassert>
#include "DAO.h"
#include "BusinessLayer/User.h"
#include "BusinessLayer/Binding.h"
#include "BusinessLayer/Message.h"

DAO::DAO():
    _maindb(QSqlDatabase::addDatabase("QSQLITE")),
    _inited(false)
{
#ifndef NDEBUG
    qDebug() << "DAO created";
#endif
}

DAO::~DAO()
{
#ifndef NDEBUG
    qDebug() << "DAO deleted";
#endif
}

void DAO::initDB() const
{
    if (!QFile("maindb.sqlite").exists())
    {
        loadDBfilled();
    }
    else
    {
        loadDB();
    }

    _inited = true;
}

void DAO::loadDB() const
{
    _maindb.setDatabaseName("maindb.sqlite");

    if (!_maindb.open())
    {
        Logger::getInstance().writeError("Cannot open the database");
    }
}

void DAO::loadDBfilled() const
{
    loadDB();

    _maindb.exec("CREATE TABLE `Users` \
                 ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, \
                 `username` TEXT NOT NULL UNIQUE, \
                 `bio` TEXT, `email` TEXT NOT NULL UNIQUE, \
                 `pass` TEXT NOT NULL, \
                 `hashkey` TEXT NOT NULL )");

    _maindb.exec("CREATE TABLE `Bindings` \
                 ( `ida` INTEGER NOT NULL, \
                 `idb` INTEGER NOT NULL, \
                 FOREIGN KEY(`idb`) REFERENCES `Users`(`id`) \
                 ON DELETE CASCADE, \
                 PRIMARY KEY(`ida`,`idb`), \
                 FOREIGN KEY(`ida`) REFERENCES `Users`(`id`) \
                 ON DELETE CASCADE )");

    _maindb.exec("CREATE TABLE `Messages` \
                 ( `ida` INTEGER NOT NULL, \
                 `idb` INTEGER NOT NULL, \
                 `text` TEXT NOT NULL, \
                 `v_from` INTEGER NOT NULL, \
                 `v_when` INTEGER NOT NULL, \
                 FOREIGN KEY(`idb`) REFERENCES `Users`(`id`) \
                 ON DELETE CASCADE, \
                 PRIMARY KEY(`ida`,`idb`,`v_when`), \
                 FOREIGN KEY(`ida`) REFERENCES `Users`(`id`) \
                 ON DELETE CASCADE )");

#ifndef NDEBUG
    qDebug() << "The data table created";
#endif
}

bool DAO::addUser(const User& user) const
{
    return QSqlQuery(_maindb)
            .exec(QString("INSERT INTO Users (username, bio, email, pass, hashkey) \
                           VALUES ('%1', '%2', '%3', '%4', '%5')")
                         .arg(user.username(),
                              user.bio() == Q_NULLPTR ? "-" : user.bio(),
                              user.email(),
                              user.pass(),
                              user.hashkey()));
}

bool DAO::addBinding(const Binding& binding) const
{
    return QSqlQuery(_maindb)
            .exec(QString("INSERT INTO Bindings (ida, idb) \
                           VALUES (%1, %2)")
                         .arg(QString::number(binding.ida()),
                              QString::number(binding.idb())));
}

bool DAO::addMessage(const Message& message) const
{
    return QSqlQuery(_maindb)
            .exec(QString("INSERT INTO Messages (ida, idb, text, v_from, v_when) \
                           VALUES (%1, %2, '%3', %4, %5)")
                         .arg(QString::number(message.ida()),
                              QString::number(message.idb()),
                              message.text(),
                              QString::number(message.from()),
                              QString::number(message.when())));
}

User DAO::getUserByUsername(const QString& username) const
{
    QSqlQuery result = _maindb.exec(QString("SELECT * \
                                             FROM Users \
                                             WHERE username = '%1'")
                              .arg(username));
    if (result.next())
    {
        return User(result.value(0).toString().toUInt(),
                    result.value(1).toString(), result.value(2).toString(),
                    result.value(3).toString(), result.value(4).toString(),
                    result.value(5).toString());
    }
    else
    {
        return User();
    }
}

QList<QString> DAO
    ::getMessagesBetwUsers(const QString& usr0, const QString& usr1) const
{
    QSqlQuery result = _maindb.exec(QString("SELECT username, v_when, text \
                                            FROM Messages INNER JOIN Users \
                                                 ON Messages.v_from = Users.id \
                                            WHERE ida IN (SELECT id \
                                                          FROM Users \
                                                          WHERE username = '%1' \
                                                             OR username = '%2') \
                                              AND idb IN (SELECT id \
                                                          FROM Users \
                                                          WHERE username = '%1' \
                                                             OR username = '%2') \
                                            ORDER BY v_when")
                                    .arg(usr0, usr1));
    QList<QString> data;
    while (result.next())
    {
        data.append(result.value(0).toString() + " ["
                  + getTime(result.value(1).toString()) + "]\n"
                  + result.value(2).toString());
    }

    return data;
}

QList<QString> DAO::getUserBindings(const QString& username) const
{
    QSqlQuery result = _maindb.exec(QString("SELECT username \
                                            FROM Users \
                                            WHERE id IN (SELECT ida \
                                                         FROM Bindings INNER JOIN Users \
                                                              ON Bindings.idb = Users.id \
                                                         WHERE username = '%1') \
                                            UNION \
                                            SELECT username \
                                            FROM Users \
                                            WHERE id IN (SELECT idb \
                                                         FROM Bindings INNER JOIN Users \
                                                              ON Bindings.ida = Users.id \
                                                         WHERE username = '%1')")
                                    .arg(username));
    QList<QString> data;
    while (result.next())
    {
        data.append(result.value(0).toString());
    }

    return data;
}

QString getTime(const QString& unixSecs)
{
    QDateTime dt = QDateTime::fromTime_t(unixSecs.toUInt());
    QString textdate = dt.toString(Qt::TextDate);
    return textdate;
}
