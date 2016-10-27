#include "BluedHdf5Converter.h"
#include "DataManager.h"
#include "BluedInputSource.h"
#include "H5Cpp.h"

using namespace H5;

void BluedHdf5Converter::convertToHdf5(const std::string& input_file, const std::string& output_file)
{
    DefaultDataManager mgr;
    BluedInputSource input;
    input.readWholeLocation(input_file, mgr);

    try {
        this->createNewDataSpace(output_file);
        this->writeData(mgr);
    } catch(...) {

    }
    input.stopReading();
    mgr.getDataPoints(10000);
    
    input.stopGracefully();


}
void BluedHdf5Converter::writeData(DefaultDataManager& mgr)
{
    hsize_t offset[2] = {0,0};
    hsize_t hyper_slab_dimensions[2] = { 1, 2};

    DataSpace mspace (rank, hyper_slab_dimensions);

    for(int i = 0; i<12000*5;++i) {
        auto data_point = mgr.getDataPoints(1);
        hsize_t size[2] = {offset[0] + 1, this->dims[1]};
        dataset.extend( size );

        DataSpace fspace = dataset.getSpace ();

        fspace.selectHyperslab( H5S_SELECT_SET, hyper_slab_dimensions, offset );
        void* data_ptr =& data_point[0];
        dataset.write( (void*) &data_ptr, PredType::NATIVE_FLOAT,mspace , fspace );

        this->offset[0] += 1;
    }
    
}

void BluedHdf5Converter::createNewDataSpace(const std::string& output_file)
{

    H5std_string  FILE_NAME(output_file.c_str());
    H5std_string  DATASET_NAME("IntArray");

    file = H5File(FILE_NAME, H5F_ACC_TRUNC);


    DataType datatype(PredType::NATIVE_FLOAT);


    hsize_t max_dims[ndims] = {H5S_UNLIMITED, ncols};

    this->file_data_space = DataSpace(rank, this->dims, max_dims);
    DSetCreatPropList cparms;
    hsize_t chunk_dims[rank] = {chunk_size, ncols};
    cparms.setChunk(rank, chunk_dims);

    this->dataset = file.createDataSet(DATASET_NAME, datatype, file_data_space, cparms);
}


