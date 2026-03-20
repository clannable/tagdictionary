#include "taglistwidgetitem.h"

TagListWidgetItem::TagListWidgetItem(QString tagPath, QListWidget* parent) :
    QListWidgetItem(parent),
    value(tagPath)
{
    int rstart = tagPath.lastIndexOf("/");
    setText(rstart == -1 ? tagPath : tagPath.right(tagPath.size() - (rstart+1)));
    setToolTip(value);
}

QString TagListWidgetItem::getValue() {
    return value;
}
