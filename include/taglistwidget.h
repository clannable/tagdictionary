#ifndef TAGLISTWIDGET_H
#define TAGLISTWIDGET_H

#include <QListWidget>
#include <nlohmann/json.hpp>
#include "tagnode.h"
#include "tagtree.h"
#include <qevent.h>
#include <list>

using json = nlohmann::json;

class TagListWidget : public QListWidget
{
    Q_OBJECT

public:
    TagListWidget(QWidget *parent);

    bool contains(json tag);
    std::list<std::string> values();

    void linkTagTree(const TagTree* ptr);

public slots:
    void setEditMode(bool mode);
    void insertTag(TagNode* node);
    void insertTag(QString tagPath);
    void setTag(TagNode* node);

protected:
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private:
    const TagTree* tagTree = nullptr;
    bool editModeEnabled = false;
    QListWidgetItem *selectedItem = nullptr;
    QListWidgetItem *menuItem = nullptr;

    TagNode* currentTag = nullptr;
    void onRemoveTag();
};

#endif // TAGLISTWIDGET_H
