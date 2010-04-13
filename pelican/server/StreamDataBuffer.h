#ifndef STREAMDATABUFFER_H
#define STREAMDATABUFFER_H

#include "DataBuffer.h"
#include <QQueue>
#include <QObject>

/**
 * @file StreamDataBuffer.h
 */

namespace pelican {

class LockableStreamData;
class Data;
class DataManager;
class LockedData;
class WritableData;

/**
 * @class StreamDataBuffer
 *
 * @brief
 * Maintain StreamData and memory access.
 *
 * @details
 * Encapsulates memory allocation for streams, with locking and data
 * consistency checking.
 */
class StreamDataBuffer : public DataBuffer
{
    Q_OBJECT

    private:
        friend class StreamDataBufferTest;

    public:
        /// StreamDataBuffer constructor.
        /// FIXME: Is data manager ever not null other than in tests?
        StreamDataBuffer(const QString& type, DataManager* manager = NULL,
                         const size_t max = 10240,
                         const size_t maxChunkSize = 10240,
                         QObject* parent = 0);

        /// StreamDataBuffer destructor.
        ~StreamDataBuffer();

        /// Get the next data object that is ready to be served.
        void getNext(LockedData&);

        /// Get a data object that is ready to be written to.
        WritableData getWritable(size_t size);

        /// Set the data manager to use.
        void setDataManager(DataManager* manager) {_manager = manager;}

        /// Sets the maximum size of the buffer in bytes.
        void setMaxSize(const size_t size) { _max = size; }

        /// Sets the maximum chunk size of the in bytes.
        void setMaxChunkSize(const size_t size) { _maxChunkSize = size; }

    protected slots:
        /// Places the data chunk that emitted the signal on the serve queue.
        void activateData();

        /// Places the data chunk that emitted the signal on the empty queue.
        void deactivateData();

    protected:
        /// Places the given data chunk on the serve queue.
        void activateData(LockableStreamData*);

        /// Places the given data chunk on the empty queue.
        void deactivateData(LockableStreamData*);
        LockableStreamData* _getWritable(size_t size);

    private:
        StreamDataBuffer(const StreamDataBuffer&); // Disallow copying.

    private:
        size_t _max;
        size_t _maxChunkSize;
        size_t _space;
        QList<LockableStreamData*> _data; ///< List of all allocated memory blocks.
        QQueue<LockableStreamData*> _serveQueue; ///< Queue of blocks waiting to be served.
        QList<LockableStreamData*> _emptyQueue; ///< List of all available blocks.
        DataManager* _manager;
};

} // namespace pelican

#endif // STREAMDATABUFFER_H