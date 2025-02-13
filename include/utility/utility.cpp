#include "utility/utility.hpp"
#include <cstdarg>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// Function to get the parameters from the JSON file
float get_param(const std::string &process, const std::string &param) {
    std::ifstream config_file("../config/drone_parameters.json");
    if (!config_file) {
        std::cerr
            << "Error opening the config file /config/drone_parameters.json"
            << std::endl;
        logging("ERROR",
                "Error opening the config file /config/drone_parameters.json");
        return EXIT_FAILURE;
    }

    std::stringstream buffer;
    buffer << config_file.rdbuf();
    std::string jsonContent = buffer.str();
    config_file.close();

    cJSON *json = cJSON_Parse(jsonContent.c_str());
    if (!json) {
        std::cerr << "Error parsing JSON file" << std::endl;
        logging("ERROR",
                "Error parsing the config file /config/drone_parameters.json");
        return EXIT_FAILURE;
    }

    cJSON *process_obj = cJSON_GetObjectItem(json, process.c_str());
    if (!process_obj) {
        std::cerr << "Process not found: " << process << std::endl;
        logging("ERROR", ("Error process not found: " + process).c_str());
        cJSON_Delete(json);
        return -1;
    }

    cJSON *param_obj = cJSON_GetObjectItem(process_obj, param.c_str());
    if (!param_obj || !cJSON_IsNumber(param_obj)) {
        std::cerr << "Parameter not found or not a number: " << param
                  << std::endl;
        logging(
            "ERROR",
            ("Error parameter not found or not a number: " + param).c_str());
        cJSON_Delete(json);
        return -1;
    }

    float value = static_cast<float>(param_obj->valuedouble);
    cJSON_Delete(json);
    return value;
}

// Function to write log messages in the logfile
void logging(const std::string &type, const std::string &message) {
    std::ofstream logfile(LOGFILE_PATH, std::ios::app);
    if (logfile) {
        logfile << "[" << type << "] - " << message << std::endl;
    }
}

// Max function for several values
int max_of_many(int count, ...) {
    va_list args;
    va_start(args, count);
    int max_val = va_arg(args, int);
    for (int i = 1; i < count; i++) {
        int value = va_arg(args, int);
        if (value > max_val) {
            max_val = value;
        }
    }
    va_end(args);
    return max_val;
}

void tokenization(std::vector<pos> &arr_to_fill, std::string to_tokenize,
                  int &objects_num) {
    size_t index_of = to_tokenize.find(']');
    if (index_of == std::string::npos)
        return;

    std::string tokens = to_tokenize.substr(index_of + 1);
    std::stringstream ss(tokens);
    std::string token;

    int index = 0;
    while (std::getline(ss, token, '|')) {
        float aux_x, aux_y;
        sscanf(token.c_str(), "%f,%f", &aux_x, &aux_y);
        arr_to_fill.push_back({aux_x, aux_y});
        logging("INFO", token);
        index++;
    }
    objects_num = index;
}

void remove_target(int index, std::vector<pos> &objects_arr) {
    if (index >= 0 && index < static_cast<int>(objects_arr.size())) {
        objects_arr.erase(objects_arr.begin() + index);
    }
}

void signal_handler(int signo, siginfo_t *info, void *context) {
    (void)(context);
    if (signo == SIGUSR1) {
        pid_t WD_pid = info->si_pid;
        Kill(WD_pid, SIGUSR2);
    }
}
