#include "TestStreamAdapter.h"
#include "pelican/data/test/TestDataBlob.h"

#include "pelican/utility/memCheck.h"

namespace pelican {

using test::TestDataBlob;

/**
 * @details
 * Deserialises the data from the given QIODevice into a
 * TestDataBlob structure.
 *
 * @param[in] in    A pointer to the input device.
 */
void TestStreamAdapter::deserialise(QIODevice* in)
{
    TestDataBlob* blob = static_cast<TestDataBlob*>(_data);
    blob->resize(_chunkSize);
    in->read( blob->data().data(), (quint64)_chunkSize);
}

} // namespace pelican
