#include "include/Generated/TargetPubSubTypes.hpp"
#include <csignal>
#include <ctime>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif
#include "constants.h"
#include "include/wrappers/wrappers.h"
#include "include/utility/utility.h"
#ifdef __cplusplus
}
#endif

using namespace eprosima::fastdds::dds;

struct TargetMessage {
    double target_x[N_TARGETS];
    double target_y[N_TARGETS];
};

class TargetPublisher {
  public:
    TargetPublisher()
        : participant_(nullptr), publisher_(nullptr), topic_(nullptr),

          writer_(nullptr), type_(new TargetPubSubType()) {}

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
        topic_ = participant_->create_topic(TOPIC_NAME_TARGET,
                                            type_.get_type_name(), tqos);
        if (!topic_)
            return false;

        DataWriterQos wqos;
        writer_ = publisher_->create_datawriter(topic_, wqos, nullptr);
        return writer_ != nullptr;
    }

    void publish(const TargetMessage &message) { writer_->write(&message); }

    ~TargetPublisher() {
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

void signal_handler(int signo, siginfo_t *info, void *context) {
    pid_t WD_pid = -1;
    // Specifying that context is unused
    (void)(context);

    if (signo == SIGUSR1) {
        WD_pid = info->si_pid;
        kill(WD_pid, SIGUSR2);
    }
}

// Function to write log messages in the logfile
void logging(const char *type, const char *message) {
    FILE *F;
    F = fopen(LOGFILE_PATH, "a");
    // Locking the logfile
    flock(fileno(F), LOCK_EX);
    fprintf(F, "[%s] - %s\n", type, message);
    // Unlocking the file so that the server can access it again
    flock(fileno(F), LOCK_UN);
    fclose(F);
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    TargetPublisher publisher;
    if (!publisher.init()) {
        std::cerr << "Failed to initialize Fast DDS publisher" << std::endl;
        return 1;
    }

    // Seeding the random number generator
    srandom((unsigned int)time(NULL));

    while (true) {
        TargetMessage message;
        for (int i = 0; i < N_TARGETS; i++) {
            message.target_x[i] = random() % SIMULATION_WIDTH;
            message.target_y[i] = random() % SIMULATION_HEIGHT;
        }
        publisher.publish(message);
        logging("INFO", "Target process generated a new set of targets");
        sleep(20); // Adjust as needed to control publish rate
    }

    return 0;
}
