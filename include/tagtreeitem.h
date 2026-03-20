#ifndef TAGTREEITEM_H
#define TAGTREEITEM_H

#include <QTreeWidgetItem>
#include "tagnode.h"
#include <nlohmann/json.hpp>

class TagTreeItem : public QTreeWidgetItem
{
public:
    TagTreeItem(TagNode* node);

    TagNode* getNode() const;

    void setNode(TagNode* node);
    void setKey(QString str);

    void jsonUpdated();

private:
    TagNode* node;

    bool operator<(const TagTreeItem &other)const {
        int column = treeWidget()->sortColumn();
        return text(column).toLower() < other.text(column).toLower();
    }
};

#endif // TAGTREEITEM_H
