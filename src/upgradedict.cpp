#include "upgradedict.h"

json dict_upgrade::upgradeDict(json dict) {
    json ret = json(dict);
    if (!dict.contains("tags") || !dict.contains("version")) {
        ret = json({{"tags", ret }, {"version", 1.0}});
    }
    double version = ret["version"].get<double>();

    if (version == dict_upgrade::LATEST_VERSION) return ret;

    if (version == 1.0)
        ret.update({
            {"sublists", json::array({
                "Related Tags",
                "Required Tags",
                "Frequently Tagged With"
                "Excludes Tags"
            })},
            {"version", 1.01}
        });

    for (auto itr = ret["tags"].begin(); itr != ret["tags"].end(); itr++)
        processTagRecursive(*itr, migrateSublists);

    return ret;
}

void dict_upgrade::processTagRecursive(json tag, void (*func)(json)) {
    func(tag);
    for (auto itr = tag["children"].begin(); itr != tag["children"].end(); itr++)
        processTagRecursive(*itr, func);
}

void dict_upgrade::migrateSublists(json tag) {
    json sublists = json();

    if (!tag["related"].empty())
        sublists["related_tags"] = tag["related"];

    if (!tag["required"].empty())
        sublists["required_tags"] = tag["required"];

    tag.erase("related");
    tag.erase("required");
}
