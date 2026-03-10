#include "jsonnode.h"


JsonNode::JsonNode() {
    this->data = json();
    this->key = "";
    this->parent = nullptr;
}

JsonNode::JsonNode(json data, std::string key, JsonNode* parent) {
    this->data = data;
    this->key = key;
    this->parent = parent;
    if (data.contains("children") && !data["children"].empty()) {
        for (auto& ch : data["children"].items()) {
            children[ch.key()] = new JsonNode(ch.value(), ch.key(), this);
        }

    }
}

JsonNode::~JsonNode() {
    for(const auto& [k, c] : children)
        delete c;
    this->data.~json();
}
json JsonNode::getData() const {
    return this->data;
}

std::string JsonNode::getKey() const {
    return key;
}

std::map<std::string, JsonNode*> JsonNode::getChildren() {
    return this->children;
}

JsonNode* JsonNode::getParent() const {
    return this->parent;
}

std::string JsonNode::getFullPath() const {
    if (parent == nullptr) return "";
    if (this->parent->getKey() == "") return this->key;
    return this->parent->getFullPath() + "/" + key;
}

void JsonNode::addChild(JsonNode* node) {
    this->children[node->getKey()] = node;
}

bool JsonNode::hasChild(std::string key) {
    return this->children.find(key) != children.end();
}

void JsonNode::removeChildAt(std::string index) {
    children.erase(index);
}

void JsonNode::insertChildAt(std::string index, JsonNode* child) {
    children[index] = child;
}

void JsonNode::setData(json data) {
    this->data = data;
    // this->children.clear();
    // updateChildren();
}

void JsonNode::setParent(JsonNode* parent) {
    if (this->parent != nullptr) {
        this->parent->removeChildAt(this->key);
    }
    this->parent = parent;
    this->parent->insertChildAt(this->key, this);
}

void JsonNode::setKey(std::string key) {
    if (this->parent != nullptr) {
        this->parent->removeChildAt(this->key);
        this->parent->insertChildAt(key, this);
    }
    this->key = key;
}
void JsonNode::updateChildren() {
    if (data.contains("children") && !data["children"].empty()) {
        for (auto& [k, v] : data["children"].items()) {
            this->children[k] = new JsonNode(v, k, this);
        }
    }
}



QString JsonNode::getString(std::string index) {
    return QString::fromStdString(data[index].get<std::string>());
}
json JsonNode::toJson() {
    json ret = data;

    if (!children.empty()) {
        ret["children"] = json();
        for (auto& [k, c] : children) {
            ret["children"][k] = c->toJson();
        }
    }
    return ret;
}

JsonNode* JsonNode::createRoot(json data) {
    JsonNode* root = new JsonNode(data);
    for (auto& ch : data.items()) {
        root->children[ch.key()] = new JsonNode(ch.value(), ch.key(), root);
    }
    return root;
}
