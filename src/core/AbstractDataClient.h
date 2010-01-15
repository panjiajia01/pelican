#ifndef ABSTRACTDATACLIENT_H
#define ABSTRACTDATACLIENT_H

#include <QHash>
#include <QString>

/**
 * @file AbstractDataClient.h
 */

namespace pelican {

class DataBlob;
class DataRequirements;

/**
 * @class AbstractDataClient
 *  
 * @brief
 * This is the abstract base class for the data client used by the pipeline
 * driver.
 * 
 * @details
 * The data client fetches data from the data server and makes it available
 * to the pipelines via the pipeline driver. The PipelineApplication creates
 * the appropriate data client object based on the supplied configuration.
 *
 * Inherit this class and implement the getData() method to create a new data
 * client type.
 */
class AbstractDataClient
{
    public:
        /// Data client constructor.
        AbstractDataClient();

        /// Data client destructor (virtual).
        virtual ~AbstractDataClient();

        /// Gets the requested data from the data server.
        virtual QHash<QString, DataBlob*> getData(const DataRequirements&) = 0;
};

} // namespace pelican

#endif // ABSTRACTDATACLIENT_H