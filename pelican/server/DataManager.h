#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QtCore/QString>
#include <QtCore/QHash>
#include "pelican/server/StreamDataBuffer.h"
#include "pelican/server/ServiceDataBuffer.h"
#include "pelican/server/LockedData.h"
#include "pelican/data/DataRequirements.h"
#include "pelican/utility/Config.h"

/**
 * @file DataManager.h
 */

namespace pelican {

class Data;
class StreamData;

/**
 * @class DataManager
 *
 * @brief
 * Main class for accessing data in the server.
 *
 * @details
 * Class to control access to the internal server data to ensure
 * locking etc.
 *
 * Configuration:
 * Each stream has its own section with the tag of the stream name.
 * Buffer sizes are set with the  buffer tags
 * e.g.
 *
 * <MyStream>
 *      <buffer maxSize="10240"> tags
 * </MyStream>
 */
class DataManager
{
    private:
        QHash<QString, StreamDataBuffer*> _streams;
        QHash<QString, ServiceDataBuffer*> _service;
        DataRequirements _specs;

    public:
        /// DataManager constructor.
        DataManager(const Config* config, const QString section = QString("server"));
        DataManager(const Config* config, const Config::TreeAddress& base );

        /// DataManager destructor.
        virtual ~DataManager();

        /// Associate service data
        void associateServiceData(LockableStreamData* data);

        /// Returns the data types handled by this manager.
        const DataRequirements& dataSpec() const {return _specs;}

        /// Return a list of Stream Data objects corresponding
        //  to a DataRequirement object
        QList<LockedData> getDataRequirements(const DataRequirements& req);

        /// Return the next unlocked data block from Stream Data.
        /// If the associate data requested is unavailable,
        /// LockedData will be invalid.
        LockedData getNext(const QString& type, const QSet<QString>& associateData);

        /// Return the next unlocked data block from Stream Data.
        LockedData getNext(const QString& type);

        /// Return the requested Service Data.
        LockedData getServiceData(const QString& type, const QString& version);

        /// Set up a stream buffer for the specified type.
        StreamDataBuffer* getStreamBuffer(const QString& type);

        /// Set up a service buffer for the specified type.
        ServiceDataBuffer* getServiceBuffer(const QString& type);

        /// Return a WritableData object that represents
        /// a space in the buffer of a minimum size specified
        /// An invalid Writable object will be returned if
        /// the space is not available.
        WritableData getWritableData(const QString& type, size_t size);

        /// Add a service data type to be managed.
        void setServiceDataBuffer(const QString& name, ServiceDataBuffer* buffer);

        /// Add a stream data type to be managed.
        void setStreamDataBuffer(const QString& name, StreamDataBuffer* buffer);

    public:
        const Config* _config;
        Config::TreeAddress _bufferConfigBaseAddress;
};

} // namespace pelican

#endif // DATAMANAGER_H
