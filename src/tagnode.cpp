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
    TAGS[m_id] = this;

    setDescription(data.value("description", ""));
    setIcon(data.value("icon", ""));
    setRelated(data.value("related", json::array()));
    setRequired(data.value("required", json::array()));
    setFrequent(data.value("frequent", json::array()));
    setFiles(data.value("files", list<string>()));

    if (data.contains("children")) {
        for (const auto& [k, c] : data["children"].items()) {
            TagNode* child = new TagNode(c, k);
            child->setParent(this);
        }
    }
}

bool TagNode::isRoot() const { return m_parent == nullptr; }

TagNode::~TagNode() {
    TAGS.erase(m_id);
    for(const TagNode* c : m_children)
        delete c;
}

void TagNode::setKey(string key) {
    m_key = key;
    if (m_leaf != nullptr) m_leaf->setText(0, QString::fromStdString(key));
}
void TagNode::setKey(QString key) {
    m_key = key.toStdString();
    if (m_leaf != nullptr) m_leaf->setText(0, key);
}

void TagNode::setIcon(string icon) {
    m_icon = icon;
    if (m_leaf != nullptr) m_leaf->setIcon(0, QIcon(QString::fromStdString(icon)));
}
void TagNode::setIcon(QString icon) {
    m_icon = icon.toStdString();
    if (m_leaf != nullptr) m_leaf->setIcon(0, QIcon(icon));
}

void TagNode::setDescription(string description) { m_description = description; }
void TagNode::setDescription(QString description) { m_description = description.toStdString(); }

void TagNode::setRelated(list<int> related) { m_related = related; }

void TagNode::setRequired(list<int> required) { m_required = required; }

void TagNode::setFrequent(list<int> frequent) { m_frequent = frequent; }

void TagNode::setFiles(list<string> files) { m_files = files; }
void TagNode::setFiles(QStringList files) {
    list<string> _files = {};
    for (const QString f : files) _files.push_back(f.toStdString());
    m_files = _files;
}
void TagNode::addFile(string file) { m_files.push_back(file); }

void TagNode::setParent(TagNode* parent) {
    if (m_parent == parent) return;
    if (m_parent != nullptr) m_parent->removeChild(this);
    parent->addChild(this);
    m_parent = parent;
}

void TagNode::addChild(TagNode* child) {
    const auto itr = std::find_if(m_children.begin(), m_children.end(),
                                  [child](TagNode* c) { return *c == *child; });
    if (itr != m_children.end()) return;

    m_children.push_back(child);
}

void TagNode::removeChild(TagNode* child) {
    const auto itr = std::find_if(m_children.begin(), m_children.end(),
                                  [child](TagNode* c) { return *c == *child; });
    if (itr == m_children.end()) return;
    m_children.erase(itr);
}

string TagNode::getFullPath() {
    if (isRoot()) return "";
    string path = m_parent->getFullPath();
    path += "/";
    path += m_key;
    return path;
}

void TagNode::setLeaf(QTreeWidgetItem* leaf) {
    m_leaf = leaf;
}

json TagNode::toJson() {
    json ret;
    if (isRoot()) {
        ret = json();
        for (TagNode* child : m_children)
            ret[child->key()] = child->toJson();
    }
    else {
        ret = json({
            { "id", m_id },
            { "description", m_description },
            { "icon", m_icon },
            { "related", m_related },
            { "required", m_required },
            { "frequent", m_frequent },
            { "files", m_files },
            { "children", json() }
        });
        for (TagNode* child : m_children)
            ret["children"][child->key()] = child->toJson();
    }
    return ret;
}

void TagNode::cleanSublists() {
    for (auto itr = m_related.begin(); itr != m_related.end(); itr++) {
        if (TAGS.find(itr->get<int>()) == TAGS.end())
            m_related.erase(itr--);
    }
    for (auto itr = m_required.begin(); itr != m_required.end(); itr++) {
        if (TAGS.find(itr->get<int>()) == TAGS.end())
            m_required.erase(itr--);
    }
    for (auto itr = m_frequent.begin(); itr != m_frequent.end(); itr++) {
        if (TAGS.find(itr->get<int>()) == TAGS.end())
            m_frequent.erase(itr--);
    }
}

// void TagNode::convertSublistsToId() {
//     if (!isRoot()) {

//         std::list<int> related_ids = {};
//         std::list<int> required_ids = {};

//         for (json& el : m_related)
//             related_ids.push_back(TAG_PATH_MAP[el.get<string>()]);

//         for (json& el : m_required)
//             required_ids.push_back(TAG_PATH_MAP[el.get<string>()]);


//         m_related = related_ids;
//         m_required = required_ids;
//     }

//     for (TagNode* child : m_children)
//         child->convertSublistsToId();
// }

