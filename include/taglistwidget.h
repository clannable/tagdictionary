#ifndef TAGLISTWIDGET_H
#define TAGLISTWIDGET_H

#include <QListWidget>
#include <nlohmann/json.hpp>
#include "jsonnode.h"
#include "tagtree.h"
#include <qevent.h>
using json = nlohmann::json;

class TagListWidget : public QListWidget
{
    Q_OBJECT

public:
    TagListWidget(QWidget *parent);

    bool contains(json tag);
    QList<std::string> values();
    QList<json> tags();

    void linkTagTree(const TagTree* ptr);

public slots:
    void setEditMode(bool mode);
    void insertTag(JsonNode* node);
    void insertTag(QString tagPath);


protected:
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;



private:

    const TagTree* tagTree = nullptr;
    bool editModeEnabled;
    QListWidgetItem *selectedItem;
    QListWidgetItem *menuItem;
    void onRemoveTag();
};

#endif // TAGLISTWIDGET_H
