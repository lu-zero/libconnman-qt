/*
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "clockmodel.h"

#define CONNMAN_SERVICE "net.connman"
#define CONNMAN_CLOCK_INTERFACE CONNMAN_SERVICE ".Clock"

#define SET_CONNMAN_PROPERTY(key, val) \
        if (!mClockProxy) { \
            qCritical("ClockModel: SetProperty: not connected to connman"); \
        } else { \
            QDBusPendingReply<> reply = mClockProxy->SetProperty(key, QDBusVariant(val)); \
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this); \
            connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), \
                    this, SLOT(setPropertyFinished(QDBusPendingCallWatcher*))); \
        }

ClockModel::ClockModel() :
    mClockProxy(0)
{
    QTimer::singleShot(0,this,SLOT(connectToConnman()));
}

void ClockModel::connectToConnman()
{
    if (mClockProxy && mClockProxy->isValid())
        return;

    mClockProxy = new ClockProxy(CONNMAN_SERVICE, "/", QDBusConnection::systemBus(), this);

    if (!mClockProxy->isValid()) {
        qCritical("ClockModel: unable to connect to connman");
        delete mClockProxy;
        mClockProxy = NULL;
        return;
    }

    QDBusPendingReply<QVariantMap> reply = mClockProxy->GetProperties();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,
            SLOT(getPropertiesFinished(QDBusPendingCallWatcher*)));

    connect(mClockProxy,
            SIGNAL(PropertyChanged(const QString&, const QDBusVariant&)),
            this,
            SLOT(propertyChanged(const QString&, const QDBusVariant&)));
}

void ClockModel::getPropertiesFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;
    if (reply.isError()) {
        qCritical() << "ClockModel: getProperties: " << reply.error().name() << reply.error().message();
    } else {
        QVariantMap properties = reply.value();

        Q_ASSERT(properties.contains("Timezone"));
        Q_ASSERT(properties.value("Timezone").type() == QVariant::String);
        mTimezone = properties.value("Timezone").toString();
        emit timezoneChanged();

        Q_ASSERT(properties.contains("TimezoneUpdates"));
        Q_ASSERT(properties.value("TimezoneUpdates").type() == QVariant::String);
        mTimezoneUpdates = properties.value("TimezoneUpdates").toString();
        emit timezoneUpdatesChanged();

        Q_ASSERT(properties.contains("TimeUpdates"));
        Q_ASSERT(properties.value("TimeUpdates").type() == QVariant::String);
        mTimeUpdates = properties.value("TimeUpdates").toString();
        emit timeUpdatesChanged();

        Q_ASSERT(properties.contains("Timeservers"));
        Q_ASSERT(properties.value("Timeservers").type() == QVariant::StringList);
        mTimeservers = properties.value("Timeservers").toStringList();
        emit timeserversChanged();
    }
    call->deleteLater();
}

void ClockModel::setPropertyFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<> reply = *call;
    if (reply.isError()) {
        qCritical() << "ClockModel: setProperty: " << reply.error().name() << reply.error().message();
    }
    call->deleteLater();
}

void ClockModel::propertyChanged(const QString &name, const QDBusVariant &value)
{
    if (name == "Timezone") {
        Q_ASSERT(value.variant().type() == QVariant::String);
        mTimezone = value.variant().toString();
        emit timezoneChanged();
    } else if (name == "TimezoneUpdates") {
        Q_ASSERT(value.variant().type() == QVariant::String);
        mTimezoneUpdates = value.variant().toString();
        emit timezoneUpdatesChanged();
    } else if (name == "TimeUpdates") {
        Q_ASSERT(value.variant().type() == QVariant::String);
        mTimeUpdates = value.variant().toString();
        emit timeUpdatesChanged();
    } else if (name == "Timeservers") {
        Q_ASSERT(value.variant().type() == QVariant::StringList);
        mTimeservers = value.variant().toStringList();
        emit timeserversChanged();
    }
}

QString ClockModel::timezone() const
{
    return mTimezone;
}

void ClockModel::setTimezone(const QString &val)
{
    SET_CONNMAN_PROPERTY("Timezone", val);
}

QString ClockModel::timezoneUpdates() const
{
    return mTimezoneUpdates;
}

void ClockModel::setTimezoneUpdates(const QString &val)
{
    SET_CONNMAN_PROPERTY("TimezoneUpdates", val);
}

QString ClockModel::timeUpdates() const
{
    return mTimeUpdates;
}

void ClockModel::setTimeUpdates(const QString &val)
{
    SET_CONNMAN_PROPERTY("TimeUpdates", val);
}

QStringList ClockModel::timeservers() const
{
    return mTimeservers;
}

void ClockModel::setTimeservers(const QStringList &val)
{
    SET_CONNMAN_PROPERTY("Timeservers", val);
}

void ClockModel::setDate(QDate date)
{
    QDateTime toDate(date, QTime::currentTime());
    quint64 secsSinceEpoch = (quint64)toDate.toTime_t();
    SET_CONNMAN_PROPERTY("Time", secsSinceEpoch);
}

void ClockModel::setTime(QTime time)
{
    QDateTime toDate(QDate::currentDate(), time);
    quint64 secsSinceEpoch = (quint64)toDate.toTime_t();
    SET_CONNMAN_PROPERTY("Time", secsSinceEpoch);
}
