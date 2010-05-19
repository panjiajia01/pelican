#include "pelican/core/DirectStreamDataClient.h"
#include "pelican/utility/Config.h"
#include "pelican/utility/FactoryConfig.h"
#include "pelican/utility/FactoryGeneric.h"
#include "pelican/core/DataClientFactory.h"
#include "pelican/adapters/AbstractAdapter.h"
#include "pelican/data/DataBlob.h"
#include "pelican/data/RealData.h"
#include "pelican/server/test/TestUdpChunker.h"

#include <iostream>
#include <QCoreApplication>
#include <QTime>
#include <QString>
#include <QList>
#include <QHash>

using namespace pelican;

void printData(RealData* data)
{
    for (unsigned i = 0; i < data->size(); ++i) {
        std::cout << "Data " << data->ptr()[i] << std::endl;
    }
}

int main(int argc, char** argv)
{
    try {
    QCoreApplication app(argc, argv);

    // Get command line arguments.
    if (argc != 5) {
        std::cerr << "Usage: directClient <port> <host> <chunk size, bytes> <iterations>" << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);
    QString host(argv[2]);
    int chunkSize = atoi(argv[3]);
    int iterations = atoi(argv[4]);

    QString pipelineXml = ""
            "<buffers>"
            "   <RealData>"
            "       <buffer maxSize=\"2000000\" maxChunkSize=\"2000000\"/>"
            "   </RealData>"
            "</buffers>"
            "<chunkers>"
            "    <TestUdpChunker name=\"a\">"
            "       <connection host=\"" + host + "\" port=\"" + QString::number(port) + "\"/>"
            "       <data type=\"RealData\" chunkSize=\"" + QString::number(chunkSize) + "\"/>"
            "    </TestUdpChunker>"
            "</chunkers>"
            "<clients>"
            "    <DirectStreamDataClient>"
            "        <data type=\"RealData\" adapter=\"AdapterRealData\"/>"
            "    </DirectStreamDataClient>"
            "</clients>"
            "<adapters>"
            "</adapters>";
    Config config;
    config.setFromString(pipelineXml);

    // Create the adapter factory.
    FactoryConfig<AbstractAdapter> adapterFactory(&config, "pipeline", "adapters");

    // Create the data client factory.
    DataClientFactory clientFactory(&config, "pipeline", "clients", &adapterFactory);

    // Create the data blob factory.
    FactoryGeneric<DataBlob> blobFactory;

    // Create a list of data requirements.
    QString dataType = "RealData";
    DataRequirements req;
    QList<DataRequirements> requirements;
    req.addStreamData(dataType);
    requirements.append(req);

    // Create the client.
    DirectStreamDataClient* client = static_cast<DirectStreamDataClient*>(
            clientFactory.create("DirectStreamDataClient", requirements));
    client->addStreamChunker("TestUdpChunker", "a");

    // Set up the data hash.
    QHash<QString, DataBlob*> dataHash;
    dataHash.insert(dataType, blobFactory.create(dataType));

    // Initialise loop counter.
    QTime timer;
    timer.start();
    int counter = 0;
    real_t initValue, value;
    for (counter = 0; counter < iterations; counter++) {
        // Get the data.
        QHash<QString, DataBlob*> validData = client->getData(dataHash);

        // Check the content of the data blob.
        RealData* realData = (RealData*)validData.value(dataType);
        //printData(realData);
        value = realData->ptr()[0];
        if (counter == 0)
            initValue = value;
    }

    // Check for lost packets.
    double dataRange = value - initValue;
    double lostPackets = dataRange + 1 - iterations;

    // Compute bandwidth.
    double sec = (double)timer.elapsed() / 1e3;
    double megaBytesReceived = (long double)chunkSize * iterations / (1024 * 1024);

    // Print summary.
    std::cout << "---------------------------------------------------------\n";
    std::cout << "Data range " << dataRange << " over " << iterations
            << " iterations. (Missed " << lostPackets << " packets.)\n";
    std::cout << "Received " << megaBytesReceived << " MiB in "
            << sec << " seconds.\n";
    std::cout << "Bandwidth: " << ((megaBytesReceived * 8) / sec)
            << " Mb/sec" << std::endl;
    std::cout << "---------------------------------------------------------\n";

    } catch (QString error) {
        std::cerr << error.toStdString() << std::endl;
    }

    return 0;
}