#ifndef UTILS_H
#define UTILS_H

typedef enum {
  SUCCESS = 1,
  NOT_A_FILE = 0,
  CANNOT_DETERMINE_PATH = -1
} FindDataFileResult;

FindDataFileResult find_data_file(char *basename, char **path);

#endif // UTILS_H
