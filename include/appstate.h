#ifndef APPSTATE_H
#define APPSTATE_H

#include <QObject>
#include "tagnode.h"

class AppState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool editModeEnabled READ editModeEnabled WRITE setEditModeEnabled NOTIFY editModeChanged)
    Q_PROPERTY(TagNode* selectedTag READ selectedTag WRITE setSelectedTag NOTIFY selectedTagChanged)
public:
    AppState(QObject* parent = nullptr);

    bool editModeEnabled() { return m_editModeEnabled; }
    void setEditModeEnabled(bool editModeEnabled);
    void toggleEditMode();

    TagNode* selectedTag() { return m_selectedTag; }
    void setSelectedTag(TagNode* selectedTag);
    bool isSelectedTagEditable();
    void signalSelectedTagUpdated();

signals:
    void editModeChanged(bool editModeEnabled);
    void selectedTagChanged(TagNode* selectedTag);
    void selectedTagUpdated();

private:
    bool m_editModeEnabled = false;
    TagNode* m_selectedTag = nullptr;
};

#endif // APPSTATE_H
