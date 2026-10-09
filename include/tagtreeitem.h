#ifndef TAGTREEITEM_H
#define TAGTREEITEM_H

#include <QTreeWidgetItem>
#include "tagnode.h"
#include <nlohmann/json.hpp>

class TagTreeItem : public QTreeWidgetItem
{
public:
    TagTreeItem(TagNode* tag);

    TagNode* tag() const { return m_tag; }
    void setTag(TagNode* tag);

    void refreshFileIcons();

private:
    TagNode* m_tag;

    bool operator<(const TagTreeItem &other)const {
        int column = treeWidget()->sortColumn();
        return text(column).toLower() < other.text(column).toLower();
    }


};

#endif // TAGTREEITEM_H
