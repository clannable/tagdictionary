#ifndef UPGRADEDICT_H
#define UPGRADEDICT_H

#include <list>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;



namespace dict_upgrade {

double LATEST_VERSION = 1.01;

json upgradeDict(json dict);

void processTagRecursive(json tag, void (*func)(json));

void migrateSublists(json tag);

};

#endif // UPGRADEDICT_H
