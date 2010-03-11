#ifndef TESTPIPELINECALIBRATEIMAGE_H
#define TESTPIPELINECALIBRATEIMAGE_H

#include "core/AbstractPipeline.h"
#include <QString>
#include <QHash>

/**
 * @file TestPipelineCalibrateImage.h
 */

namespace pelican {

class DataBlob;
class ZenithImagerDft;
class ImageWriterFits;
class ZenithModelVisibilities;
class ZenithCalibrater;

class ModelVisibilityData;
class CorrectedVisibilityData;
class ImageData;

/**
 * @class TestPipelineCalibrateImage
 *
 * @brief
 *
 * @details
 */

class TestPipelineCalibrateImage : public AbstractPipeline
{
    public:
        TestPipelineCalibrateImage();
        ~TestPipelineCalibrateImage();

    public:
        /// Creates the required modules
        void init();

        /// Run method for single iteration of the pipeline
        void run(QHash<QString, DataBlob*>& data);

    private:
        ZenithImagerDft* _imager;
        ZenithModelVisibilities* _modelGen;
        ImageWriterFits* _fitsWriter;
        ZenithCalibrater* _calibrate;

        ModelVisibilityData* _modelVis;
        CorrectedVisibilityData* _correctedVis;
        ImageData* _calImage;
        ImageData* _rawImage;
        ImageData* _psfImage;
        ImageData* _modelImage;
};

} // namespace pelican

#endif // TESTPIPELINECALIBRATEIMAGE_H