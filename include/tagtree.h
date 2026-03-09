#ifndef TAGTREE_H
#define TAGTREE_H

#include <QTreeWidget>
#include <QMouseEvent>
#include "./jsonnode.h"
#include "./tagtreeitem.h"
#include <nlohmann/json.hpp>

class TagTree : public QTreeWidget
{
    Q_OBJECT

public:
    TagTree(QWidget* parent);

    TagTreeItem* findTag(QString tagPath);

    void fromJson(nlohmann::json json);
    json toJson();

public slots:
    void setEditMode(bool mode);
    void onCreateTag();
    void onNewTag(JsonNode* node);
    void onRemoveTag();
protected:

    // virtual void mouseMoveEvent(QMouseEvent *event) override;
    // virtual void mousePressEvent(QMouseEvent *event) override;
    // virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void dropEvent(QDropEvent *event) override;

signals:
    void addToRelated(JsonNode *node);
    void addToRequired(JsonNode *node);
    void tagsChanged();

private:
    JsonNode* rootNode;
    // TagTreeItem* dragItem;
    TagTreeItem* menuItem;

    // QPoint dragStartPosition;
    QMap<QModelIndex,JsonNode*> nodeMap;
    bool editModeEnabled;
    TagTreeItem* findTag(TagTreeItem* item, QString tagPath);

    void signalRelated();
    void signalRequired();

    void createChildren(TagTreeItem* item, JsonNode *node);
};

#endif // TAGTREE_H
