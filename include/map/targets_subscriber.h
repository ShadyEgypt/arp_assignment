#ifndef TARGETS_SUBSCRIBER_H
#define TARGETS_SUBSCRIBER_H

#include "Generated/src/map/TargetMessagePubSubTypes.hpp"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <iostream>
#include <atomic>
#include <csignal>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

#endif // TARGETS_SUBSCRIBER_H
