#ifndef TAGTREEITEM_H
#define TAGTREEITEM_H

#include <QTreeWidgetItem>
#include "./jsonnode.h"
#include <nlohmann/json.hpp>

class TagTreeItem : public QTreeWidgetItem
{
public:
    TagTreeItem(JsonNode* node);

    JsonNode* getNode() const;
    json getJson() const;

    void setNode(JsonNode* node);
    void setKey(QString str);

    void jsonUpdated();



private:
    JsonNode* node;

    bool operator<(const TagTreeItem &other)const {
        int column = treeWidget()->sortColumn();
        return text(column).toLower() < other.text(column).toLower();
    }
};

#endif // TAGTREEITEM_H
