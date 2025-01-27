#include "utils/utils.h"

// Function to get the parameters from the JSON file
float get_param(const char *process, const char *param) {

    FILE *config_file;
    char jsonBuffer[1000];
    char logmsg[300];

    // Open the config file
    // The relative path is used and the executable is in the bin folder
    config_file = fopen("../config/drone_parameters.json", "r");
    if (config_file == NULL) {
        perror("Error opening the config file /config/drone_parameters.json");
        sprintf(
            logmsg,
            "Error opening the config file /config/drone_parameters.json\n ");
        logging(LOG_ERROR, logmsg);
        return EXIT_FAILURE; // 1
    }
    fread(jsonBuffer, 1, sizeof(jsonBuffer), config_file);
    fclose(config_file);

    // Parse the JSON content
    cJSON *json = cJSON_Parse(jsonBuffer);

    if (json == NULL) {
        perror("Error parsing JSON file\n");
        sprintf(
            logmsg,
            "Error parsing the config file /config/drone_parameters.json\n ");
        logging(LOG_ERROR, logmsg);
        return EXIT_FAILURE;
    }

    // Navigate to the specified process
    cJSON *process_obj = cJSON_GetObjectItem(json, process);
    if (!process_obj) {
        printf("Process not found: %s\n", process);
        sprintf(logmsg, "Error process not found: %s\n", process);
        logging(LOG_ERROR, logmsg);
        cJSON_Delete(json);
        return -1;
    }

    // Retrieve the specified parameter
    cJSON *param_obj = cJSON_GetObjectItem(process_obj, param);
    if (!param_obj || !cJSON_IsNumber(param_obj)) {
        printf("Parameter not found or not a number: %s\n", param);
        sprintf(logmsg, "Error parameter not found or not a number: %s\n",
                param);
        logging(LOG_ERROR, logmsg);
        cJSON_Delete(json);
        return -1;
    }

    float value = (float)param_obj->valuedouble;
    cJSON_Delete(json);
    return value;
}

// Function to write log messages in the logfile
void logging(char *type, char *message) {
    FILE *F;
    F = Fopen(LOGFILE_PATH, "a");
    // Locking the logfile
    Flock(fileno(F), LOCK_EX);
    fprintf(F, "[%s] - %s\n", type, message);
    // Unlocking the file so that the server can access it again
    Flock(fileno(F), LOCK_UN);
    Fclose(F);
}

// basic max function
int max(int a, int b) {
    if (a >= b)
        return a;
    return b;
}

void tokenization(struct pos *arr_to_fill, char *to_tokenize,
                  int *objects_num) {
    int index_of;
    char *char_pointer;
    char *aux_ptr;
    char *token;

    // Take the index of the first character ']' appearing in the string
    // to_tokenize
    char_pointer = strchr(to_tokenize, ']');
    index_of     = (int)(char_pointer - to_tokenize);

    // Start the tokenization loop after ']' so +1 and split on '|'
    token     = strtok_r(to_tokenize + index_of + 1, "|", &aux_ptr);
    int index = 1;
    if (token != NULL) {
        float aux_x, aux_y;
        index = 1;
        // Take x and y of the new target or obstacle
        sscanf(token, "%f,%f", &aux_x, &aux_y);
        arr_to_fill[0].x = aux_x;
        arr_to_fill[0].y = aux_y;
        // Log the processed token
        logging(LOG_INFO, token);
        // Does the same thing as before but this time is put in a loop to
        // process the whole string
        while ((token = strtok_r(NULL, "|", &aux_ptr)) != NULL) {
            sscanf(token, "%f,%f", &aux_x, &aux_y);
            logging(LOG_INFO, token);
            arr_to_fill[index].x = aux_x;
            arr_to_fill[index].y = aux_y;
            index++;
        }
    }
    *objects_num = index;
}

void remove_target(int index, struct pos *objects_arr, int objects_num) {
    // Removes the target or obstacle at index from the target_arr
    for (int i = index; i < objects_num - 1; i++) {
        objects_arr[i].x = objects_arr[i + 1].x;
        objects_arr[i].y = objects_arr[i + 1].y;
    }
}

void signal_handler(int signo, siginfo_t *info, void *context) {
    pid_t WD_pid = -1;
    // Specifying that context is unused
    (void)(context);

    if (signo == SIGUSR1) {
        WD_pid = info->si_pid;
        Kill(WD_pid, SIGUSR2);
    }
}
