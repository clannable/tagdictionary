#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>
#include <QListWidgetItem>
#include "tagnode.h"
#include "taglist.h"
#include <nlohmann/json.hpp>

namespace Ui {
class TagEditor;
}

class TagEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TagEditor(QWidget *parent = nullptr);
    ~TagEditor();

public slots:
    void onEditModeChange(bool editModeEnabled);
    void selectIcon();
    void iconSelected(QString icon);
    void setTag(TagNode *tag);
    void save();
    // void onListItemSelect(int tagId);
    // void addToRelated(TagNode* node);
    // void addToRequired(TagNode* node);
    // void refreshLists();

private slots:
    void onAnchorClick(const QUrl &link);

signals:
    // void editModeChanged(bool mode);
    void tagSaved(TagNode* tag);
    // void partialSave(TagNode* tag);
    // void listItemSelected(int tagId);
    void displayLinkClicked(int index);

private:
    Ui::TagEditor *ui;

    TagList* u_requiredList;
    TagList* u_relatedList;
    TagList* u_frequentList;

    QString m_iconPath;

    static QFrame* createHLine();
};

#endif // TAGEDITOR_H
