#ifndef OBJ_LOADHER_H
#define OBJ_LOADHER_H

#include "platform.h"
#include "vector.h"
#include "strings.h"

struct Model;

void load_obj(String file_path, Model* model);

#endif