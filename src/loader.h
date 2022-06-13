#ifndef LOADER_H
#define LOADER_H

#include <glib.h>

#include "utils.h"

GPtrArray *read_emojis_from_file(const char *path);

#endif // LOADER_H
