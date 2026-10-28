#ifndef TAGLIST_H
#define TAGLIST_H

#include <QWidget>
#include <QMouseEvent>
#include <QMimeData>
#include "nlohmann/json.hpp"
#include <QTreeWidget>
#include "tokenparser.h"

class TagList : public QTreeWidget
{
    enum Modifier {
        NONE = 0,
        IF_THEN = 1,
        AND = 2,
        OR = 3,
        SUB = 4,
    };

    Q_OBJECT

public:
    TagList(QWidget* parent = nullptr);
    std::list<std::string> values();

    void removeTag(int id);
    void setValues(nlohmann::json::array_t ids);
signals:
    void tagRemoved();

public slots:
    void onEditModeChanged(bool editModeEnabled);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual QMimeData* mimeData(const QList<QTreeWidgetItem*>& items) const override;
    virtual QStringList mimeTypes() const override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

protected slots:
    void onItemDoubleClicked(QTreeWidgetItem* item);

private:
    QString itemToString(QTreeWidgetItem* item) const;
    void insertChildTag(QTreeWidgetItem* item, QTreeWidgetItem* parent, Modifier mod, int index = -1);
    TokenParser* parser;
};

#endif // TAGLIST_H
