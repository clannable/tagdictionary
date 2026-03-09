#ifndef TAGLISTWIDGETITEM_H
#define TAGLISTWIDGETITEM_H

#include <QListWidgetItem>

class TagListWidgetItem : public QListWidgetItem
{
public:
    TagListWidgetItem(QString tagPath, QListWidget* parent=nullptr);

    QString getValue();
private:
    QString value;
};

#endif // TAGLISTWIDGETITEM_H
