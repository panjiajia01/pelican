#ifndef CHUNKERMANAGER_H
#define CHUNKERMANAGER_H

#include <QSet>
#include <QMap>
#include <QString>
#include <QPair>
#include <QVector>
#include <boost/shared_ptr.hpp>

#include "pelican/utility/FactoryConfig.h"
#include "pelican/server/DataReceiver.h"
#include "pelican/utility/Config.h"

/**
 * @file ChunkerManager.h
 */

namespace pelican {

class AbstractChunker;
class DataManager;

/**
 * @class ChunkerManager
 *
 * @brief
 * Manage and operate chunkers.
 *
 * @details
 * Anything coming through a stream data chunker
 * will be associated with the current version of service data
 * service data has no such association.
 */
class ChunkerManager
{
    public:
        /// Creates a chunker manager.
        /// @deprecated
        ChunkerManager(const Config* config,
                const QString& section = QString("server"));  /* deprecated */

        /// Creates a chunker manager.
        ChunkerManager(const Config* config,  const Config::TreeAddress& base );

        /// Destroys the chunker manager.
        ~ChunkerManager();

        /// Initialise the passed data manager to support the contained chunkers.
        void init(DataManager& dm);

        /// Add a stream chunker.
        void addStreamChunker(QString type, QString name = QString());

        /// Add a service chunker.
        void addServiceChunker(QString type, QString name = QString());

        /// get a list of chunkers
        const QSet<AbstractChunker* >& chunkers() const { return _chunkers; };

        /// returns true if all the data chunkers are operating/flase otherwise
        bool isRunning() const;

    private:
        /// Add an allocated chunker.
        void _addChunker( AbstractChunker* chunker);

    private:
        FactoryConfig<AbstractChunker> *_factory;
        QMap<QPair<QString,quint16>,AbstractChunker* > _chunkerPortMap;
        QSet<AbstractChunker* > _chunkers;
        QSet<QString> _streamDataTypes;
        QSet<QString> _serviceDataTypes;
        QVector<DataReceiver*> _dataReceivers;
};

} // namespace pelican

#endif // CHUNKERMANAGER_H
