#ifndef DATABLOBEXAMPLE_H
#define DATABLOBEXAMPLE_H

/**
 * @file DataBlobExample.h
 */

#include "pelican/data/DataBlob.h"
#include <vector>

using namespace pelican;

/**
 * @class DataBlobExample
 *
 * @brief
 * Data blob to hold an array of real-valued data.
 *
 * @details
 * This data blob holds an array of real-valued data.
 */
class DataBlobExample : public DataBlob
{
    private:
        std::vector<float> _data;

    public:
        /// Constructor.
        DataBlobExample() : DataBlob() {}

        /// Returns a pointer to the start of the data.
        float* ptr() {return (_data.size() > 0 ? &_data[0] : NULL);}

        /// Resizes the data blob.
        void resize(int length) {_data.resize(length);}

        /// Returns the size of the data.
        unsigned size() {return _data.size();}
};

#endif // DATABLOBEXAMPLE_H
