#include "tagnode.h"


TagNode::TagNode() {
    this->root = this;
}
TagNode::TagNode(json data, string key, TagNode* parent) {
    this->parent = parent;
    this->key = key;
    this->updateFullPath();
    this->description = data.value("description", "");
    this->icon = data.value("icon", "");
    if (this->parent != nullptr)
        this->root = this->parent->getRoot();
    if (data.contains("children") && !data["children"].empty()) {
        for (auto& ch : data["children"].items())
            children[ch.key()] = new TagNode(ch.value(), ch.key(), this);
    }
    if (data.contains("related") && !data["related"].empty())
        this->related = data["related"].get<list<string>>();
    if (data.contains("required") && !data["required"].empty())
        this->required = data["required"].get<list<string>>();


}

TagNode* TagNode::createRoot(json data) {
    TagNode* root = new TagNode();
    for (auto& ch : data.items()) {
        root->children[ch.key()] = new TagNode(ch.value(), ch.key(), root);
    }
    return root;
}


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
    bool changed = key != this->key;
    this->key = key;
    if (changed)
        this->updateFullPath();


}
void TagNode::setKey(QString key) { this->setKey(key.toStdString()); }

TagNode* TagNode::getRoot() const { return this->root; }

string TagNode::getIcon() const { return this->icon; }
void TagNode::setIcon(string icon) { this->icon = icon; }
void TagNode::setIcon(QString icon) { this->icon = icon.toStdString(); }


string TagNode::getDescription() const { return this->description; }
void TagNode::setDescription(string description) { this->description = description; }
void TagNode::setDescription(QString description) { this->description = description.toStdString(); }


list<string> TagNode::getRelated() const { return this->related; }
void TagNode::setRelated(list<string> related) { this->related = related; }


list<string> TagNode::getRequired() const { return this->required; }
void TagNode::setRequired(list<string> required) { this->required = required; }


list<string> TagNode::getFiles() const { return this->files; }
void TagNode::setFiles(list<string> files) { this->files = files; }
void TagNode::addFile(string file) { this->files.push_back(file); }


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
void TagNode::updateFullPath(PathChanges* changes) {
    string path = "";
    bool rootChange = changes == nullptr;
    if (rootChange)
        changes = new PathChanges();

    if (this->parent != nullptr)
        path = this->parent->getFullPath();
    if (!path.empty())
        path += "/";
    path += this->key;
    if (path != this->fullPath && !this->fullPath.empty())
        changes->push_back({this->fullPath, path});
    this->fullPath = path;

    for (auto& [k, c] : this->children)
        c->updateFullPath(changes);

    if (rootChange && !changes->empty())
        this->root->renameListEntries(*changes);
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

void TagNode::renameListEntries(PathChanges changes) {
    if (!related.empty()) {
        for (auto ci = changes.begin(); ci != changes.end(); ++ci) {
            auto [oldPath, newPath] = *ci;
            auto it = find(related.begin(), related.end(), oldPath);
            if (it != related.end()) {
                related.erase(it);
                if (!newPath.empty())
                    related.push_back(newPath);
            }
        }

    }
    if (!required.empty()) {
        for (auto ci = changes.begin(); ci != changes.end(); ++ci) {
            auto [oldPath, newPath] = *ci;
            auto it = find(required.begin(), required.end(), oldPath);
            if (it != required.end()) {
                required.erase(it);
                if (!newPath.empty())
                    required.push_back(newPath);
            }
        }
    }

    for (const auto& [k, c] : children)
        c->renameListEntries(changes);
}


json TagNode::toJson() {
    json ret = json({
     { "description", this->description },
     { "icon", this->icon },
     { "related", this->related },
     { "required", this->required }
    });

    if (!children.empty()) {
        json childrenJson = json();
        for (auto& [k, c] : children)
            childrenJson.emplace(k, c->toJson());
        ret.emplace("children", childrenJson);
    }
    return ret;
}

