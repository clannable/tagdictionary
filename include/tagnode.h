#ifndef TAGNODE_H
#define TAGNODE_H

#include <list>
#include <nlohmann/json.hpp>
#include <QString>
#include <QTreeWidgetItem>

using json = nlohmann::json;
using namespace std;

typedef vector<pair<string, string>> PathChanges;
class TagNode
{

public:
    TagNode();
    TagNode(json data, string key="");

    static TagNode* createRoot(json data);

    ~TagNode();

    bool isRoot() const;

    int id() const { return m_id; }

    string description() const { return m_description; }
    void setDescription(string description);
    void setDescription(QString description);

    string icon() const { return m_icon; }
    void setIcon(string icon);
    void setIcon(QString icon);

    string key() const { return m_key; }
    void setKey(string key);
    void setKey(QString key);

    list<string> files() const { return list<string>(m_files); }
    void setFiles(list<string> files);
    void setFiles(QStringList files);
    void addFile(string file);

    json::array_t related() const { return m_related; }
    void setRelated(list<int> related);

    json::array_t required() const { return m_required; }
    void setRequired(list<int> required);

    json::array_t frequent() const { return m_frequent; }
    void setFrequent(list<int> frequent);

    void cleanSublists();

    list<TagNode*> children() const { return list<TagNode*>(m_children); }
    void addChild(TagNode* child);
    void removeChild(TagNode* child);
    void insertChild(TagNode* child);

    TagNode* parent() const;
    void setParent(TagNode* parent);

    // void convertSublistsToId();

    string getFullPath();

    json toJson();

    QTreeWidgetItem* leaf() const { return m_leaf; }
    void setLeaf(QTreeWidgetItem* leaf);

    bool operator==(TagNode &rhs) const { return m_id == rhs.m_id; }
    bool operator!=(TagNode &rhs) const { return m_id != rhs.m_id; }

private:
    int m_id = -1;
    string m_key = "";
    string m_description = "";
    string m_icon = "";
    list<string> m_files;
    json m_related = json::array();
    json m_required = json::array();
    json m_frequent = json::array();

    TagNode* m_parent = nullptr;

    list<TagNode*> m_children = {};
    QTreeWidgetItem* m_leaf = nullptr;
};

#endif // TAGNODE_H
