#ifndef SMART_SCREEN_CROSSVALIDATIONSUCCESS_H
#define SMART_SCREEN_CROSSVALIDATIONSUCCESS_H

#include <EventMetaData.h>
#include <DefaultDataPoint.h>
#include <boost/serialization/map.hpp>
#include <boost/serialization/serialization.hpp>


struct CrossValidationSuccess {
    int classified_correctly = 0;
    int classified_incorrectly = 0;
    std::map<EventMetaData::LabelType, std::map<EventMetaData::LabelType,int>> broken_down;

    template<class Archive> void serialize(Archive &ar,  const unsigned int version) {
        ar & classified_correctly;
        ar & classified_incorrectly;
        ar & broken_down;
    }
};
#endif //SMART_SCREEN_CROSSVALIDATIONSUCCESS_H
