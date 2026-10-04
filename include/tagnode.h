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

    bool isRoot() const;

    int getId() const;

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

    json getRelated() const;
    void setRelated(json related);

    json getRequired() const;
    void setRequired(json required);

    map<string, TagNode*> getChildren();
    TagNode* getParent() const;
    void setParent(TagNode* parent);

    string getFullPath() const;
    void updateFullPath();

    void addChild(TagNode* node);
    bool hasChild(string key);
    void removeChildAt(string key);
    void insertChildAt(string key, TagNode* child);

    bool hasImages();
    bool hasVideos();

    void convertSublistsToId();

    json toJson();

private:
    int id = 0;
    string key = "";
    string description = "";
    string icon = "";
    list<string> files;
    json related;
    json required;
    string fullPath = "";

    TagNode* parent = nullptr;
    TagNode* root = nullptr;
    map<string, TagNode*> children;

    bool wImages = false;
    bool wVideos = false;

    void checkFiles();
};

#endif // TAGNODE_H
