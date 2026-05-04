#ifndef HANDLE_INPUT_H
#define HANDLE_INPUT_H

struct OsEvent;
template<typename T> struct Array;

void update_input();
bool handle_input(Array<OsEvent>* events);

#endif