#include "tagnode.h"
#include <regex>
#include "globals.h"

TagNode::TagNode() {}

TagNode::TagNode(json data, string key, TagNode* parent) {
    this->parent = parent;
    if (data.contains("id") && !isRoot()) {
        this->id = data["id"].get<int>();
        if (this->id >= NEXT_TAG_ID)
            NEXT_TAG_ID = this->id+1;
    } else if (!isRoot()) {
        this->id = NEXT_TAG_ID;
        NEXT_TAG_ID++;
    }
    TAG_MAP[this->id] = this;
    this->key = key;

    updateFullPath();

    this->description = data.value("description", "");
    this->icon = data.value("icon", "");

    if (data.contains("children") && !data["children"].empty()) {
        for (auto& ch : data["children"].items())
            children[ch.key()] = new TagNode(ch.value(), ch.key(), this);
    }
    if (data.contains("related") && !data["related"].empty())
        this->related = data["related"];
    if (data.contains("required") && !data["required"].empty())
        this->required = data["required"];
    if (data.contains("frequent") && !data["frequent"].empty())
        this->frequent = data["frequent"];
    if (data.contains("files") && !data["files"].empty()) {
        this->files = data["files"].get<list<string>>();
        this->checkFiles();
    }

}

TagNode* TagNode::createRoot(json data) {
    TagNode* root = new TagNode();
    for (auto& ch : data.items()) {
        root->children[ch.key()] = new TagNode(ch.value(), ch.key(), root);
    }
    return root;
}

int TagNode::getId() const { return this->id; }

bool TagNode::isRoot() const { return parent == nullptr; }

TagNode::~TagNode() {

    for(const auto& [k, c] : children)
        delete c;
}


string TagNode::getKey() const { return key; }
void TagNode::setKey(string key) {
    if (this->parent != nullptr) {
        this->parent->removeChildAt(this->key);
        this->parent->insertChildAt(key, this);
    }
    this->key = key;
}
void TagNode::setKey(QString key) { this->setKey(key.toStdString()); }

string TagNode::getIcon() const { return this->icon; }
void TagNode::setIcon(string icon) { this->icon = icon; }
void TagNode::setIcon(QString icon) { this->icon = icon.toStdString(); }


string TagNode::getDescription() const { return this->description; }
void TagNode::setDescription(string description) { this->description = description; }
void TagNode::setDescription(QString description) { this->description = description.toStdString(); }


json TagNode::getRelated() const { return this->related; }
void TagNode::setRelated(json related) { this->related = related; }


json TagNode::getRequired() const { return this->required; }
void TagNode::setRequired(json required) { this->required = required; }

json TagNode::getFrequent() const { return this->frequent; }
void TagNode::setFrequent(json frequent) { this->frequent = frequent; }

list<string> TagNode::getFiles() const { return this->files; }
void TagNode::setFiles(list<string> files) {
    this->files = files;
    checkFiles();
}
void TagNode::addFile(string file) {
    this->files.push_back(file);
    checkFiles();
}


TagNode* TagNode::getParent() const { return this->parent; }

void TagNode::setParent(TagNode* parent) {
    if (this->parent != nullptr) {
        this->parent->removeChildAt(this->key);
    }
    this->parent = parent;
    this->parent->insertChildAt(this->key, this);
    this->updateFullPath();
}

string TagNode::getFullPath() const { return this->fullPath; }

void TagNode::updateFullPath() {
    string path = "";

    if (!isRoot())
        path = this->parent->getFullPath();
    if (!path.empty())
        path += "/";
    path += this->key;

    this->fullPath = path;
    TAG_PATH_MAP[this->fullPath] = this->id;
}


map<string, TagNode*> TagNode::getChildren() { return this->children; }

void TagNode::addChild(TagNode* node) {
    this->children[node->getKey()] = node;
}

bool TagNode::hasChild(string key) {
    return this->children.find(key) != children.end();
}

void TagNode::removeChildAt(string index) {
    children.erase(index);
}

void TagNode::insertChildAt(string index, TagNode* child) {
    children[index] = child;
}

bool TagNode::hasImages() {
    return this->wImages;
}

bool TagNode::hasVideos() {
    return this->wVideos;
}

json TagNode::toJson() {
    json ret;
    if (isRoot()) {
        ret = json();
        if (!children.empty()) {
            for (auto& [k, c] : children)
                ret.emplace(k, c->toJson());
        }
    }
    else {
        ret = json({
            { "id", this->id },
            { "description", this->description },
            { "icon", this->icon },
            { "related", this->related },
            { "required", this->required },
            { "frequent", this->frequent },
            { "files", this->files }
        });

        if (!children.empty()) {
            json childrenJson = json();
            for (auto& [k, c] : children)
                childrenJson.emplace(k, c->toJson());
            ret.emplace("children", childrenJson);
        }
    }
    return ret;
}

void TagNode::checkFiles() {
    bool images = false;
    bool videos = false;

    regex videoRegex("\\.(mov|mp4|wmv)$", regex_constants::icase);
    regex imageRegex("\\.(jpeg|png|gif|jpg|bmp|jfif|webp)$", regex_constants::icase);
    for (const string& file : this->files) {
        if (!videos && regex_search(file, videoRegex))
            videos = true;
        if (!images && regex_search(file, imageRegex))
            images = true;
        if (images && videos)
            break;
    }
    wImages = images;
    wVideos = videos;
}

void TagNode::cleanSublists() {
    for (auto itr = this->related.begin(); itr != this->related.end(); itr++) {
        if (TAG_MAP.find(itr->get<int>()) == TAG_MAP.end())
            this->related.erase(itr--);
    }
    for (auto itr = this->required.begin(); itr != this->required.end(); itr++) {
        if (TAG_MAP.find(itr->get<int>()) == TAG_MAP.end())
            this->required.erase(itr--);
    }
    for (auto itr = this->frequent.begin(); itr != this->frequent.end(); itr++) {
        if (TAG_MAP.find(itr->get<int>()) == TAG_MAP.end())
            this->frequent.erase(itr--);
    }
}

void TagNode::convertSublistsToId() {
    if (!isRoot()) {

        std::list<int> related_ids = {};
        std::list<int> required_ids = {};

        for (json& el : this->related) {
            related_ids.push_back(TAG_PATH_MAP[el.get<string>()]);
        }
        for (json& el : this->required) {
            required_ids.push_back(TAG_PATH_MAP[el.get<string>()]);
        }

        this->related = related_ids;
        this->required = required_ids;
    }

    for (auto& [k, c] : children)
        c->convertSublistsToId();
}

