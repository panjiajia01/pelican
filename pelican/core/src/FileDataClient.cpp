#include "core/FileDataClient.h"
#include "adapters/AbstractServiceAdapter.h"
#include "adapters/AbstractStreamAdapter.h"
#include "adapters/AdapterFactory.h"
#include "data/DataRequirements.h"
#include "data/DataBlobFactory.h"
#include "utility/ConfigNode.h"

#include <QFile>
#include <QSet>
#include <QtGlobal>

#include "utility/memCheck.h"

namespace pelican {

/**
 * @details
 * This creates a new file data client.
 */
FileDataClient::FileDataClient(const ConfigNode& config, const DataTypes& types)
    : AbstractDataClient(config, types )
{
    // Get the configuration options.
    _getConfig();
}

/**
 * @details
 * Destroys the data client.
 */
FileDataClient::~FileDataClient()
{
}

/**
 * @details
 * Read data blobs from a file data source and fill the hash that points to
 * available data.
 */
QHash<QString, DataBlob*> FileDataClient::getData(QHash<QString, DataBlob*>& dataHash)
{
    // Create the local data hash to return.
    QHash<QString, DataBlob*> validHash;

    // Loop over each pipeline's set of data requirements.
    foreach (DataRequirements req, dataRequirements()) {

        // Loop over service data requirements.
        foreach (QString type, req.serviceData()) {
            QString filename = _fileNames.value(type);
            if (!filename.isEmpty()) {
                QFile file(filename);
                if (!file.open(QIODevice::ReadOnly))
                    throw QString("FileDataClient::getData(): "
                            "Cannot open file %1").arg(filename);
                AbstractServiceAdapter* adapter = serviceAdapter(type);
                Q_ASSERT( adapter != 0 );
                adapter->config(dataHash[type], file.size());
                adapter->deserialise(&file);
                validHash.insert(type, dataHash.value(type));
            }
        }

        // Loop over stream data requirements.
        foreach (QString type, req.streamData()) {
            QString filename = _fileNames.value(type);
            if (!filename.isEmpty()) {
                QFile file(filename);
                if (!file.open(QIODevice::ReadOnly))
                    throw QString("FileDataClient::getData(): "
                            "Cannot open file %1").arg(filename);
                AbstractStreamAdapter* adapter = streamAdapter(type);
                Q_ASSERT( adapter != 0 );
                QHash<QString, DataBlob*> serviceHash;
                adapter->config(dataHash[type], file.size(), serviceHash);
                adapter->deserialise(&file);
                validHash.insert(type, dataHash.value(type));
            }
        }
    }

    return validHash;
}

/**
 * @details
 * Gets the configuration options from the XML configuration node.
 */
void FileDataClient::_getConfig()
{
    // Get all the filenames for each data type.
    _fileNames = configNode().getOptionHash("data", "type", "file");
}

} // namespace pelican