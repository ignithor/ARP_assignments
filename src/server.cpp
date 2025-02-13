#include "Generated/TargetPubSubTypes.hpp"
#include "constants.h"
#include "droneDataStructs.h"
#include "utility/utility.hpp"
#include "wrappers/wrappers.hpp"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace eprosima::fastdds::dds;

struct TargetMessage {
    double target_x[N_TARGETS];
    double target_y[N_TARGETS];
};

class TargetSubscriber {
  public:
    TargetSubscriber(int to_drone_pipe, int to_map_pipe)
        : participant_(nullptr), subscriber_(nullptr), topic_(nullptr),
          reader_(nullptr), listener_(to_drone_pipe, to_map_pipe),
          type_(new TargetPubSubType()) {} // Match declaration order

    bool init() {
        DomainParticipantQos pqos;
        participant_ =
            DomainParticipantFactory::get_instance()->create_participant(0,
                                                                         pqos);
        if (!participant_)
            return false;

        type_.register_type(participant_);

        SubscriberQos sub_qos;
        subscriber_ = participant_->create_subscriber(sub_qos, nullptr);
        if (!subscriber_)
            return false;

        TopicQos tqos;
        topic_ = participant_->create_topic(TOPIC_NAME_TARGET,
                                            type_.get_type_name(), tqos);
        if (!topic_)
            return false;

        DataReaderQos rqos;
        reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);
        return reader_ != nullptr;
    }

    void run() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    ~TargetSubscriber() {
        if (reader_)
            subscriber_->delete_datareader(reader_);
        if (topic_)
            participant_->delete_topic(topic_);
        if (subscriber_)
            participant_->delete_subscriber(subscriber_);
        if (participant_)
            DomainParticipantFactory::get_instance()->delete_participant(
                participant_);
    }

  private:
    class SubListener : public DataReaderListener {
      public:
        SubListener(int to_drone_pipe, int to_map_pipe)
            : to_drone_pipe_(to_drone_pipe), to_map_pipe_(to_map_pipe) {}

        void on_data_available(DataReader *reader) override {
            SampleInfo info;
            TargetMessage message;
            if (reader->take_next_sample(&message, &info) ==
                eprosima::fastdds::dds::RETCODE_OK) {
                if (info.valid_data) {
                    // Process the received target message
                    logging("INFO", (char *)"Received new target data");
                    char msg_to_send[MAX_MSG_LEN] = "T["; // Message to send targets
                    char aux_to_send[MAX_MSG_LEN] = {0};
                    sprintf(aux_to_send, "[%d]", N_TARGETS);
                    strcat(msg_to_send, aux_to_send);

                    for (int i = 0; i < N_TARGETS; i++) {
                        if (i != 0)
                            strcat(msg_to_send,
                                   "|"); // Separate targets with "|"
                        // Append formatted target coordinates to message
                        sprintf(aux_to_send, "%.3f,%.3f", message.target_x[i],
                                message.target_y[i]);
                        strcat(msg_to_send, aux_to_send);
                    }
                    // Write the message to the pipes
                    Write(to_map_pipe_, msg_to_send, MAX_MSG_LEN);
                    Write(to_drone_pipe_, msg_to_send, MAX_MSG_LEN);
                }
            }
        }

      private:
        int to_drone_pipe_; // Store pipe file descriptors
        int to_map_pipe_;
    } listener_;

    DomainParticipant *participant_;
    Subscriber *subscriber_;
    Topic *topic_;
    DataReader *reader_;
    TypeSupport type_;
};

int main(int argc, char *argv[]) {
    HANDLE_WATCHDOG_SIGNALS();

    // Initialize pipes
    int from_drone_pipe, to_drone_pipe, from_input_pipe, to_input_pipe,
        from_map_pipe, to_map_pipe, from_target_pipe, to_target_pipe,
        from_obstacles_pipe, to_obstacle_pipe;

    if (argc == 11) {
        sscanf(argv[1], "%d", &from_drone_pipe);
        sscanf(argv[2], "%d", &to_drone_pipe);
        sscanf(argv[3], "%d", &from_input_pipe);
        sscanf(argv[4], "%d", &to_input_pipe);
        sscanf(argv[5], "%d", &from_map_pipe);
        sscanf(argv[6], "%d", &to_map_pipe);
        sscanf(argv[7], "%d", &from_target_pipe);
        sscanf(argv[8], "%d", &to_target_pipe);
        sscanf(argv[9], "%d", &from_obstacles_pipe);
        sscanf(argv[10], "%d", &to_obstacle_pipe);
    } else {
        printf("Server: wrong number of arguments in input\n");
        getchar();
        exit(1);
    }

    // Initialize the TargetSubscriber with pipe file descriptors
    TargetSubscriber subscriber(to_drone_pipe, to_map_pipe);
    if (!subscriber.init()) {
        std::cerr << "Failed to initialize Fast DDS subscriber" << std::endl;
        return 1;
    }

    // Run the subscriber in a separate thread
    std::thread subscriber_thread(&TargetSubscriber::run, &subscriber);

    // Structs for each drone information
    struct pos drone_current_pos           = {0, 0}; // Initialize all fields
    struct velocity drone_current_velocity = {0, 0}; // Initialize all fields

    // Declaring the logfile aux buffer
    char received[MAX_MSG_LEN];
    char to_send[MAX_MSG_LEN];

    fd_set reader;
    fd_set master;
    // Resetting the fd_sets
    FD_ZERO(&reader);
    FD_ZERO(&master);
    FD_SET(from_drone_pipe, &master);
    FD_SET(from_input_pipe, &master);
    FD_SET(from_map_pipe, &master);
    FD_SET(from_obstacles_pipe, &master);
    FD_SET(from_target_pipe, &master);
    FD_SET(from_map_pipe, &master);

    // Setting the maxfd
    int maxfd =
        max_of_many(6, from_drone_pipe, from_input_pipe, from_map_pipe,
                    from_obstacles_pipe, from_target_pipe, from_map_pipe);

    bool to_exit = false;
    while (1) {
        // perform the select
        reader = master;
        Select_wmask(maxfd + 1, &reader, NULL, NULL, NULL);

        // check the value returned by the select and perform actions
        // consequently
        for (int i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &reader)) {
                int ret = Read(i, received, MAX_MSG_LEN);
                if (ret == 0) {
                    printf("Pipe to server closed\n");
                    Close(i);
                    FD_CLR(i, &master);
                } else {
                    if (i == from_input_pipe) {
                        // If the user wants to stop the processes then forward
                        // it to all the others
                        if (!strcmp(received, (char *)"STOP")) {
                            Write(to_drone_pipe, (char *)"STOP", MAX_MSG_LEN);
                            Write(to_map_pipe, (char *)"STOP", MAX_MSG_LEN);
                            Write(to_obstacle_pipe, (char *)"STOP",
                                  MAX_MSG_LEN);
                            Write(to_target_pipe, (char *)"STOP", MAX_MSG_LEN);
                            to_exit = true;
                            break;
                        } else if (!strcmp(received, "U")) {
                            // Otherwise send the drone position and
                            // velocity calculated from the drone process to
                            // the input
                            sprintf(to_send, "%f,%f|%f,%f", drone_current_pos.x,
                                    drone_current_pos.y,
                                    drone_current_velocity.x_component,
                                    drone_current_velocity.y_component);
                            Write(to_input_pipe, to_send, MAX_MSG_LEN);
                        } else { // If the input process with force sends the
                                 // force to the drone pipe
                            Write(to_drone_pipe, received, MAX_MSG_LEN);
                        }

                    } else if (i == from_drone_pipe) {
                        // The drone process sends the update speed and position
                        // of the drone
                        sscanf(received, "%f,%f|%f,%f", &drone_current_pos.x,
                               &drone_current_pos.y,
                               &drone_current_velocity.x_component,
                               &drone_current_velocity.y_component);
                        // Send the drone current position to the map
                        sprintf(to_send, "D%f|%f", drone_current_pos.x,
                                drone_current_pos.y);
                        Write(to_map_pipe, to_send, MAX_MSG_LEN);
                    } else if (i == from_map_pipe) {
                        logging("INFO", received);
                        if (!strcmp(received, (char *)"GE")) {
                            // If we have GE sent by the map then send to target
                            // so it can produce new targets
                            Write(to_target_pipe, (char *)"GE", MAX_MSG_LEN);
                        } else if (received[0] == 'T' && received[1] == 'H') {
                            // If TH then there has been a target hit, inform
                            // the drone in order to remove it from the targets
                            // to consider for the forces calculations
                            Write(to_drone_pipe, received, MAX_MSG_LEN);
                        }
                    } else if (i == from_obstacles_pipe) {
                        // When new obstacles are ready inform map and drone
                        Write(to_map_pipe, received, MAX_MSG_LEN);
                        Write(to_drone_pipe, received, MAX_MSG_LEN);
                    } else if (i == from_target_pipe) {
                        // When new targets are ready, replace the pipe data
                        // with the data from the subscriber
                        // (This logic is now handled by the subscriber)
                    }
                }
            }
        }
        // If STOP sent then it needs to be closed
        if (to_exit)
            break;
    }
    // Close pipes
    Close(from_drone_pipe);
    Close(from_input_pipe);
    Close(from_map_pipe);
    Close(from_obstacles_pipe);
    Close(from_target_pipe);
    Close(to_drone_pipe);
    Close(to_map_pipe);
    Close(to_obstacle_pipe);
    Close(to_target_pipe);
    Close(to_input_pipe);

    // Wait for the subscriber thread to finish
    subscriber_thread.join();

    return EXIT_SUCCESS;
}