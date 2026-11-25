#include "tagnode.h"
#include "globals.h"

TagNode::TagNode() {}

TagNode* TagNode::createRoot(json data) {
    TagNode* root = new TagNode();
    for (auto& [k, c] : data.items()) {
        TagNode* child = new TagNode(c, k);
        child->setParent(root);
    }
    return root;
}

TagNode::TagNode(json data, string key) : m_key(key) {
    if (data.contains("id")) {
        m_id = data["id"].get<int>();
        if (m_id >= NEXT_TAG_ID)
            NEXT_TAG_ID = m_id+1;
    } else {
        m_id = NEXT_TAG_ID++;
    }

    setDescription(data.value("description", ""));
    setIcon(data.value("icon", ""));
    setRelated(data.value("related", json::array()));
    setRequired(data.value("required", json::array()));
    setFrequent(data.value("frequent", json::array()));
    setExcludes(data.value("excludes", json::array()));
    setFiles(data.value("files", list<string>()));
    m_children = data.value("children", list<int>());
    setReplacesParent(data.value<bool>("replacesParent", false));

    TAGS[m_id] = this;
}

//! Returns `true` if the provided node is the top-level root node, false otherwise.
//! There should only ever be one root node in the tree structure, which acts as the parent to all top-level tags. The root node has no ID or parent node.
bool TagNode::isRoot() { return m_id <= 0 && m_parent == nullptr; }

void TagNode::setKey(string key) { m_key = key; }
void TagNode::setKey(QString key) { m_key = key.toStdString(); }

void TagNode::setIcon(string icon) { m_icon = icon; }
void TagNode::setIcon(QString icon) { m_icon = icon.toStdString(); }

void TagNode::setDescription(string description) { m_description = description; }
void TagNode::setDescription(QString description) { m_description = description.toStdString(); }

void TagNode::setRelated(list<string> related) { m_related = related; }
void TagNode::setRelated(list<int> related) {
    list<string> vals = {};
    for (int i : related) vals.push_back(to_string(i));
    m_related = json(vals);
}
void TagNode::setRelated(json related) { m_related = related; }
void TagNode::setRequired(list<string> required) { m_required = required; }
void TagNode::setRequired(list<int> required) {
    list<string> vals = {};
    for (int i : required) vals.push_back(to_string(i));
    m_required = json(vals);
}
void TagNode::setRequired(json required) { m_required = required; }
void TagNode::setFrequent(list<string> frequent) { m_frequent = frequent; }
void TagNode::setFrequent(list<int> frequent) {
    list<string> vals = {};
    for (int i : frequent) vals.push_back(to_string(i));
    m_frequent = json(vals);
}
void TagNode::setFrequent(json frequent) { m_frequent = frequent; }

void TagNode::setExcludes(list<string> excludes) { m_excludes = excludes; }
void TagNode::setExcludes(list<int> excludes) {
    list<string> vals = {};
    for (int i : excludes) vals.push_back(to_string(i));
    m_excludes = json(vals);
}
void TagNode::setExcludes(json excludes) { m_excludes = excludes; }

void TagNode::setFiles(list<string> files) { m_files = files; }
void TagNode::setFiles(QStringList files) {
    list<string> _files = {};
    for (const QString f : files) _files.push_back(f.toStdString());
    m_files = _files;
}
void TagNode::addFile(string file) { m_files.push_back(file); }

//! Assigns the provided tag as this tag's parent, and adds this tag's ID to the new parent's list of children.
//! If this tag already has a parent, its ID is removed from that parent's list of children.
void TagNode::setParent(TagNode* parent) {
    if (m_parent == parent) return;
    if (m_parent) m_parent->removeChild(m_id);
    if (parent) parent->addChild(m_id);
    m_parent = parent;
}

//! Add the provided tag ID to this tag's list of children, if not already present.
//! Assigns this tag as the added tag's parent.
void TagNode::addChild(int id) {
    if (id == 0) return;
    const auto itr = std::find(m_children.begin(), m_children.end(), id);
    if (itr != m_children.end()) return;
    m_children.push_back(id);
    TAGS[id]->setParent(this);
}

void TagNode::setReplacesParent(bool replacesParent) { m_replacesParent = replacesParent; }

//! Remove a tag with the given ID from this tag's list of children, and unassign the removed tag's parent.
void TagNode::removeChild(int id) {
    const auto itr = std::find(m_children.begin(), m_children.end(), id);
    if (itr == m_children.end()) return;
    m_children.erase(itr);
    TAGS[id]->setParent(nullptr);
}

string TagNode::getFullPath() {
    if (isRoot()) return "";
    string path = m_parent->getFullPath();
    path += "/";
    path += m_key;
    return path;
}

//! Returns a JSON object representation of this tag.
json TagNode::toJson() {
    json ret = json({
        { "id", m_id },
        { "description", m_description },
        { "icon", m_icon }
    });
    if (!m_related.empty()) ret["related"] = m_related;
    if (!m_required.empty()) ret["required"] = m_required;
    if (!m_frequent.empty()) ret["frequent"] = m_frequent;
    if (!m_excludes.empty()) ret["excludes"] = m_excludes;
    if (!m_files.empty()) ret["files"] = m_files;
    if (m_replacesParent) ret["replacesParent"] = m_replacesParent;
    if (!m_children.empty()) ret["children"] = m_children;
    return ret;
}

//! Removes any tags that no longer exist from this tag's sub-lists.
void TagNode::cleanSublists() {
    for (auto itr = m_related.begin(); itr != m_related.end(); itr++) {
        int id = itr->is_string() ? TagNode::parseId(itr->get<string>()) : itr->get<int>();
        if (TAGS.find(id) == TAGS.end())
            m_related.erase(itr--);
    }
    for (auto itr = m_required.begin(); itr != m_required.end(); itr++) {
        int id = itr->is_string() ? TagNode::parseId(itr->get<string>()) : itr->get<int>();
        if (TAGS.find(id) == TAGS.end())
            m_required.erase(itr--);
    }
    for (auto itr = m_frequent.begin(); itr != m_frequent.end(); itr++) {
        int id = itr->is_string() ? TagNode::parseId(itr->get<string>()) : itr->get<int>();
        if (TAGS.find(id) == TAGS.end())
            m_frequent.erase(itr--);
    }
}
