#ifndef JSONNODE_H
#define JSONNODE_H

#include <nlohmann/json.hpp>
#include <QString>

using json = nlohmann::json;


class JsonNode
{
public:
    JsonNode();
    JsonNode(json data, std::string key="", JsonNode* parent=nullptr);

    ~JsonNode();

    json getData() const;
    std::string getKey() const;
    std::map<std::string, JsonNode*> getChildren();
    JsonNode* getParent() const;
    std::string getFullPath() const;

    void setData(json data);
    void setKey(std::string key);
    void setParent(JsonNode* parent);
    void addChild(JsonNode* node);
    bool hasChild(std::string key);
    void removeChildAt(std::string key);
    void insertChildAt(std::string key, JsonNode* child);


    QString getString(std::string index);
    json toJson();

    static JsonNode* createRoot(json data);
private:
    json data;
    std::string key;
    std::map<std::string, JsonNode*> children;
    JsonNode* parent;

    void updateChildren();
};

#endif // JSONNODE_H
