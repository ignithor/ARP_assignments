#include "Generated/ObstaclePubSubTypes.hpp"
#include "constants.h"
#include "utility/utility.hpp"
#include "wrappers/wrappers.hpp"
#include <csignal>
#include <ctime>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <iostream>

using namespace eprosima::fastdds::dds;

struct ObstacleMessage {
    double obstacle_x[N_OBSTACLES];
    double obstacle_y[N_OBSTACLES];
};

class ObstaclePublisher {
  public:
    ObstaclePublisher()
        : participant_(nullptr), publisher_(nullptr), topic_(nullptr),

          writer_(nullptr), type_(new ObstaclePubSubType()) {}

    bool init() {
        DomainParticipantQos pqos;
        participant_ =
            DomainParticipantFactory::get_instance()->create_participant(0,
                                                                         pqos);
        if (!participant_)
            return false;

        type_.register_type(participant_);

        PublisherQos pub_qos;
        publisher_ = participant_->create_publisher(pub_qos, nullptr);
        if (!publisher_)
            return false;

        TopicQos tqos;
        topic_ = participant_->create_topic(TOPIC_NAME_OBSTACLE,
                                            type_.get_type_name(), tqos);
        if (!topic_)
            return false;

        DataWriterQos wqos;
        writer_ = publisher_->create_datawriter(topic_, wqos, nullptr);
        return writer_ != nullptr;
    }

    void publish(const ObstacleMessage &message) { writer_->write(&message); }

    ~ObstaclePublisher() {
        if (writer_)
            publisher_->delete_datawriter(writer_);
        if (topic_)
            participant_->delete_topic(topic_);
        if (publisher_)
            participant_->delete_publisher(publisher_);
        if (participant_)
            DomainParticipantFactory::get_instance()->delete_participant(
                participant_);
    }

  private:
    DomainParticipant *participant_;
    Publisher *publisher_;
    Topic *topic_;
    DataWriter *writer_;
    TypeSupport type_;
};

int main() {
    HANDLE_WATCHDOG_SIGNALS();

    ObstaclePublisher publisher;
    if (!publisher.init()) {
        std::cerr << "Failed to initialize Fast DDS publisher" << std::endl;
        return 1;
    }

    // Seeding the random number generator
    srandom((unsigned int)time(NULL)*6416);
    ObstacleMessage message;
    for (int i = 0; i < N_OBSTACLES; i++) {
        message.obstacle_x[i] = random() % SIMULATION_WIDTH;
        message.obstacle_y[i] = random() % SIMULATION_HEIGHT;
    }
    time_t timestamp = time(NULL);

    ObstacleMessage message;
    for (int i = 0; i < N_OBSTACLES; i++) {
        message.obstacle_x[i] = random() % SIMULATION_WIDTH;
        message.obstacle_y[i] = random() % SIMULATION_HEIGHT;
    }
    time_t timestamp = time(NULL);


    while (true) {
        if (!PUBLISHERS_SLEEP_MODE) {
            if (difftime(time(NULL), timestamp) > OBSTACLES_SPAWN_PERIOD) {
                timestamp = time(NULL);
                for (int i = 0; i < N_OBSTACLES; i++) {
                    message.obstacle_x[i] = random() % SIMULATION_WIDTH;
                    message.obstacle_y[i] = random() % SIMULATION_HEIGHT;
                }
                logging("INFO",
                        "Obstacle process generated a new set of obstacles");
            if (difftime(time(NULL), timestamp) > OBSTACLES_SPAWN_PERIOD) {
                timestamp = time(NULL);
                for (int i = 0; i < N_OBSTACLES; i++) {
                    message.obstacle_x[i] = random() % SIMULATION_WIDTH;
                    message.obstacle_y[i] = random() % SIMULATION_HEIGHT;
                }
                logging("INFO",
                        "Obstacle process generated a new set of obstacles");
            }
            publisher.publish(message);

            sleep(500);
        }}

    return 0;
}
