/*
 * Copyright 2011 Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "components.h"

#include <networkservice.h>
#include <clockmodel.h>
#include "networkingmodel.h"
#include "technologymodel.h"

void Components::registerTypes(const char *uri)
{
    qmlRegisterUncreatableType<NetworkService>(uri,0,2,"NetworkService",
        "Please don't create \"NetworkService\" objects manually.");
    qmlRegisterType<NetworkingModel>(uri,0,2,"NetworkingModel");
    qmlRegisterType<TechnologyModel>(uri,0,2,"TechnologyModel");
    qmlRegisterType<ClockModel>(uri,0,2,"ClockModel");
}

void Components::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
    Q_UNUSED(engine);
}

Q_EXPORT_PLUGIN(Components);
