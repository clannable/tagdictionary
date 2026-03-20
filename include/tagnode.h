#ifndef TAGNODE_H
#define TAGNODE_H

#include <list>
#include <nlohmann/json.hpp>
#include <QString>

using json = nlohmann::json;
using namespace std;

typedef vector<pair<string, string>> PathChanges;
class TagNode
{
public:
    TagNode();
    TagNode(json data, string key="", TagNode* parent=nullptr);

    static TagNode* createRoot(json data);

    ~TagNode();

    TagNode* getRoot() const;

    string getDescription() const;
    void setDescription(string description);
    void setDescription(QString description);

    string getIcon() const;
    void setIcon(string icon);
    void setIcon(QString icon);

    string getKey() const;
    void setKey(string key);
    void setKey(QString key);

    list<string> getFiles() const;
    void setFiles(list<string> files);
    void addFile(string file);

    list<string> getRelated() const;
    void setRelated(list<string> related);

    list<string> getRequired() const;
    void setRequired(list<string> required);

    void renameListEntries(PathChanges changes);

    map<string, TagNode*> getChildren();
    TagNode* getParent() const;
    void setParent(TagNode* parent);

    string getFullPath() const;
    void updateFullPath(PathChanges* changes = nullptr);

    void addChild(TagNode* node);
    bool hasChild(string key);
    void removeChildAt(string key);
    void insertChildAt(string key, TagNode* child);

    json toJson();

private:
    string key = "";
    string description = "";
    string icon = "";
    list<string> files;
    list<string> related;
    list<string> required;
    string fullPath = "";

    TagNode* parent = nullptr;
    TagNode* root = nullptr;
    map<string, TagNode*> children;
};

#endif // TAGNODE_H
