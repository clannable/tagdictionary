#ifndef TAGLISTWIDGETITEM_H
#define TAGLISTWIDGETITEM_H

#include <QListWidgetItem>
#include "tagnode.h"

class TagListWidgetItem : public QListWidgetItem
{
public:
    TagListWidgetItem(QVariant tagPath, QListWidget* parent=nullptr);
    TagListWidgetItem(TagNode* node, QListWidget* parent=nullptr);

    QVariant getValue() const;

private:
    QVariant value;
    TagNode* tag;
};

#endif // TAGLISTWIDGETITEM_H
