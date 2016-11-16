#ifndef EVENTSTORAGE_H
#define EVENTSTORAGE_H

#include <vector>
#include <string>
#include <fstream>

#include <H5Cpp.h>

#include "DefaultDataPoint.h"
#include "EventMetaData.h"

class EventStorage {
public:
    template<typename IteratorType> unsigned int
    storeEvent(IteratorType begin, const IteratorType end, const EventMetaData &meta_data);


private:
    template<typename IteratorType> void
    writeToFile(IteratorType begin, const IteratorType end, const std::string &file_name,
                const EventMetaData &meta_data);

    void writeMetaDataToFile(H5::H5File &dst_file, const EventMetaData &meta_data);

    template<typename IteratorType> void
    writeEventDataToFile(IteratorType begin, const IteratorType end, H5::H5File &dst_file);

    void createEventDataSet(H5::H5File &dst_file, H5::DataSet &data_set, const hsize_t num_data_points);

private:
    enum {
        buffer_size = 256
    };

    static const int rank = 2;
    static const int ncols = 2;

    hsize_t dims[rank] = {0, ncols};

};


template<typename IteratorType> unsigned int
EventStorage::storeEvent(IteratorType begin, const IteratorType end, const EventMetaData &meta_data) {
    static unsigned int uuid = 0;
    writeToFile(begin, end, "events/event_" + std::to_string(uuid) + ".hdf5", meta_data);
    return uuid++;
}

template<typename IteratorType> void
EventStorage::writeToFile(IteratorType begin, const IteratorType end, const std::string &file_name,
                          const EventMetaData &meta_data) {
    H5::H5File file(file_name, H5F_ACC_TRUNC);
    writeMetaDataToFile(file, meta_data);
    writeEventDataToFile(begin, end, file);
    file.close();

}

inline void EventStorage::writeMetaDataToFile(H5::H5File &dst_file, const EventMetaData &meta_data) {
    using namespace H5;
    using namespace std;
    std::string time_str = boost::posix_time::to_simple_string(meta_data.event_time);
    StrType time_type(PredType::C_S1, time_str.size());
    DataSpace ds(H5S_SCALAR);
    Attribute at = dst_file.createAttribute("time", time_type, ds);
    at.write(time_type, time_str);

}

template<typename IteratorType> void
EventStorage::writeEventDataToFile(IteratorType begin, const IteratorType end, H5::H5File &dst_file) {
    DefaultDataPoint buffer[buffer_size];
    unsigned long points_written = std::min(static_cast<const unsigned long>(end - begin),
                                            static_cast<const unsigned long>( buffer_size));
    using namespace H5;

    hsize_t offset[2] = {0, 0};
    H5::DataSet dataset;
    createEventDataSet(dst_file, dataset, end-begin);

    while (points_written != 0) {
        std::copy(begin, begin + points_written, buffer);
        hsize_t size[2] = {offset[0] + points_written, this->dims[1]};
        dataset.extend(size);
        hsize_t hyper_slab_dimensions[2] = {points_written, this->dims[1]};
        DataSpace mspace(rank, hyper_slab_dimensions);
        DataSpace fspace = dataset.getSpace();

        fspace.selectHyperslab(H5S_SELECT_SET, hyper_slab_dimensions, offset);
        dataset.write(reinterpret_cast<const void *>(buffer), PredType::NATIVE_FLOAT, mspace, fspace);
        offset[0] += points_written;
        begin += points_written;

        points_written = std::min(static_cast<const unsigned long>(end - begin),
                                  static_cast<const unsigned long>( buffer_size));
    }


}

inline void EventStorage::createEventDataSet(H5::H5File &dst_file, H5::DataSet &data_set, const hsize_t num_data_points) {
    using namespace H5;
    DataType datatype(PredType::NATIVE_FLOAT);


    hsize_t max_dims[rank] = {num_data_points, ncols};

    DataSpace file_data_space(rank, this->dims, max_dims);
    DSetCreatPropList cparms;
    hsize_t chunk_dims[rank] = {buffer_size, ncols};
    cparms.setChunk(rank, chunk_dims);

    data_set = dst_file.createDataSet("/data", datatype, file_data_space, cparms);
}

#endif // EVENTSTORAGE_H
