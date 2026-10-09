#ifndef TAGLISTWIDGETITEM_H
#define TAGLISTWIDGETITEM_H

#include <QListWidgetItem>
#include "tagnode.h"

class TagListWidgetItem : public QListWidgetItem
{
public:
    TagListWidgetItem(QVariant tagPath, QListWidget* parent=nullptr);
    TagListWidgetItem(TagNode* tag, QListWidget* parent=nullptr);

    QVariant value() const { return m_value; }

private:
    QVariant m_value;
};

#endif // TAGLISTWIDGETITEM_H
