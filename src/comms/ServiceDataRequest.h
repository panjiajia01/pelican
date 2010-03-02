#ifndef SERVICEDATAREQUEST_H
#define SERVICEDATAREQUEST_H

#include <QHash>
#include "ServerRequest.h"

/**
 * @file ServiceDataRequest.h
 */

namespace pelican {

/**
 * @class ServiceDataRequest
 *  
 * @brief
 *   A request for a specific verson of service data
 * @details
 * 
 */

class ServiceDataRequest : public ServerRequest
{

    public:
        ServiceDataRequest();
        ~ServiceDataRequest();

        /// request
        //  add a request for the data object of the specified type
        //  and version
        void request(const QString& type, const QString& version);

        // return a list of data types in the request
        QList<QString> types() const;

        // return the version of a specific type
        // n.b only one version per type allowed per request
        QString version(const QString& type) const;

        // returns true if n valid requests() calls have been made
        bool isEmpty() const;

        // test for equality between ServiceData objects
        bool operator==(const ServiceDataRequest&) const;

        virtual bool operator==(const ServerRequest&) const;

    private:
        QHash<QString,QString> _dataRequested;
};

} // namespace pelican
#endif // SERVICEDATAREQUEST_H 
