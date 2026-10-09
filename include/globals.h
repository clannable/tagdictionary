#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <list>
#include "appstate.h"

extern std::list<std::string>* ICON_LIST;
extern std::string LAST_IMAGE_FOLDER_PATH;
extern std::string LAST_ICON_FOLDER_PATH;

extern int NEXT_TAG_ID;
extern std::map<int, TagNode*> TAGS;
extern std::map<std::string, int> TAG_PATH_MAP;
extern bool AUTOSAVE_ENABLED;
extern bool CONVERT_RELATED_FLAG;

extern AppState* APP_STATE;
#endif // GLOBALS_H
