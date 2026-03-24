#ifndef TAGLISTWIDGET_H
#define TAGLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <nlohmann/json.hpp>
#include "tagnode.h"
#include "tagtree.h"
#include <qevent.h>
#include <QPushButton>
#include <list>
#include <QLabel>

using json = nlohmann::json;

class TagListWidget : public QWidget
{
    Q_OBJECT

public:
    TagListWidget(QString title, QWidget *parent=nullptr);

    bool contains(json tag);
    std::list<std::string> values();

    void linkTagTree(const TagTree* ptr);

public slots:
    void setEditMode(bool mode);
    void insertTag(TagNode* node);
    void insertTag(QString tagPath);
    void setTag(TagNode* node);
    void onItemSelected(QListWidgetItem* item);
    void clear();
    void toggleExpanded();

signals:
    void tagSelected(QString tag);

protected:
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    const TagTree* tagTree = nullptr;
    bool editModeEnabled = false;
    QListWidgetItem *selectedItem = nullptr;
    QListWidgetItem *menuItem = nullptr;
    QListWidget *list;
    QLabel *countLabel;
    QWidget *header;
    QPushButton *expandToggle;
    bool expanded = false;
    QString title;
    TagNode* currentTag = nullptr;
    void onRemoveTag();

    void updateTitle();
};

#endif // TAGLISTWIDGET_H
