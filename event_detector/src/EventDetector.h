#ifndef EVENTDETECTOR_H
#define EVENTDETECTOR_H

#include "DataManager.h"
#include <string>


class EventDetector
{
public:
    void startAnalyzing(DefaultDataManager *data_manager);

private:
    void run();
private:
    DefaultDataManager *data_manager;
};

#endif // EVENTDETECTOR_H
