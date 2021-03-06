/*
 * Copyright © 2010, Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include <QDebug>
#include "technologymodel.h"
#include "useragent.h"

static const char AGENT_PATH[] = "/ConnectivitySettings";

#define CONNECT_TECHNOLOGY_SIGNALS(tech) \
    connect(tech, \
        SIGNAL(poweredChanged(bool)), \
        this, \
        SIGNAL(poweredChanged(bool))); \
    connect(tech, \
            SIGNAL(scanFinished()), \
            this, \
            SIGNAL(scanRequestFinished()))

TechnologyModel::TechnologyModel(QAbstractListModel* parent)
  : QAbstractListModel(parent),
    m_manager(NULL),
    m_tech(NULL)
{
    m_manager = NetworkManagerFactory::createInstance();

    // set default value of the "name" property
    m_techname = QString("wifi");

    QHash<int, QByteArray> roles;
    roles[ServiceRole] = "networkService";
    setRoleNames(roles);

    new UserAgent(this); // this object will be freed when TechnologyModel is freed

    m_tech = m_manager->getTechnology(m_techname);
    if (m_tech) {
        CONNECT_TECHNOLOGY_SIGNALS(m_tech);
    }

    connect(m_manager, SIGNAL(availabilityChanged(bool)),
            this, SLOT(managerAvailabilityChanged(bool)));

    connect(m_manager,
            SIGNAL(technologiesChanged()),
            this,
            SLOT(updateTechnologies()));

    connect(m_manager,
            SIGNAL(servicesChanged()),
            this,
            SLOT(updateServiceList()));

    QDBusConnection::systemBus().registerObject(AGENT_PATH, this);
    m_manager->registerAgent(QString(AGENT_PATH));
}

TechnologyModel::~TechnologyModel()
{
    m_manager->unregisterAgent(QString(AGENT_PATH));
}

QVariant TechnologyModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case ServiceRole:
        return QVariant::fromValue(static_cast<QObject *>(m_services.value(index.row())));
    }

    return QVariant();
}

int TechnologyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_services.count();
}

const QString TechnologyModel::name() const
{
    return m_techname;
}

bool TechnologyModel::isAvailable() const
{
    return m_manager->isAvailable();
}

bool TechnologyModel::isPowered() const
{
    if (m_tech) {
        return m_tech->powered();
    } else {
        qWarning() << "Can't get: technology is NULL";
        return false;
    }
}

void TechnologyModel::setPowered(const bool &powered)
{
    if (m_tech) {
        m_tech->setPowered(powered);
    } else {
        qWarning() << "Can't set: technology is NULL";
    }
}

void TechnologyModel::requestScan() const
{
    qDebug() << "scan requested for technology";
    if (m_tech) {
        m_tech->scan();
    }
}

void TechnologyModel::updateTechnologies()
{
    NetworkTechnology *test = NULL;
    if (m_tech) {
        if ((test = m_manager->getTechnology(m_techname)) == NULL) {
            // if wifi is set and manager doesn't return a wifi, it means
            // that wifi was removed
            m_tech = NULL;
        }
    } else {
        if ((test = m_manager->getTechnology(m_techname)) != NULL) {
            // if wifi is not set and manager returns a wifi, it means
            // that wifi was added
            m_tech = test;

            CONNECT_TECHNOLOGY_SIGNALS(m_tech);
        }
    }

    emit technologiesChanged();
}

void TechnologyModel::managerAvailabilityChanged(bool available)
{
    if(available)
        m_manager->registerAgent(QString(AGENT_PATH));

    emit availabilityChanged(available);
}

void TechnologyModel::requestUserInput(ServiceRequestData* data)
{
    m_req_data = data;
    emit userInputRequested(data->objectPath, data->fields);
}

void TechnologyModel::reportError(const QString &error) {
    emit errorReported(error);
}

void TechnologyModel::sendUserReply(const QVariantMap &input) {
    if (!input.isEmpty()) {
        QDBusMessage &reply = m_req_data->reply;
        reply << input;
        QDBusConnection::systemBus().send(reply);
    } else {
        QDBusMessage error = m_req_data->msg.createErrorReply(
            QString("net.connman.Agent.Error.Canceled"),
            QString("canceled by user"));
        QDBusConnection::systemBus().send(error);
    }
    delete m_req_data;
}

int TechnologyModel::indexOf(const QString &dbusObjectPath) const
{
    int idx(-1);

    foreach (NetworkService *service, m_services) {
        idx++;
        if (service->path() == dbusObjectPath) return idx;
    }

    return -1;
}

void TechnologyModel::updateServiceList()
{
    const QVector<NetworkService *> new_services = m_manager->getServices(m_techname);
    int num_new = new_services.count();

    for (int i = 0; i < num_new; i++) {
        int j = m_services.indexOf(new_services.value(i));
        if (j == -1) {
            // wifi service not found -> remove from list
            beginInsertRows(QModelIndex(), i, i);
            m_services.insert(i, new_services.value(i));
            endInsertRows();
        } else if (i != j) {
            // wifi service changed its position -> move it
            NetworkService* service = m_services.value(j);
            beginMoveRows(QModelIndex(), j, j, QModelIndex(), i);
            m_services.remove(j);
            m_services.insert(i, service);
            endMoveRows();
        }
    }

    int num_old = m_services.count();
    if (num_old > num_new) {
        beginRemoveRows(QModelIndex(), num_new, num_old - 1);
        m_services.remove(num_new, num_old - num_new);
        endRemoveRows();
    }
}
