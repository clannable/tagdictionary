#ifndef TAGLIST_H
#define TAGLIST_H

#include <QListWidget>
#include <QWidget>
#include <QMouseEvent>

class TagList : public QListWidget
{
    Q_OBJECT

public:
    TagList(QWidget* parent);

signals:
    void tagRemoved();

public slots:
    void onRemoveTag();
    void setEditMode(bool mode);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QListWidgetItem* menuItem = nullptr;
    bool editModeEnabled = false;
};

#endif // TAGLIST_H
