#include "appstate.h"

AppState::AppState(QObject* parent) : QObject(parent) {}

void AppState::setEditModeEnabled(bool editModeEnabled) {
    if (m_editModeEnabled == editModeEnabled) return;
    m_editModeEnabled = editModeEnabled;
    emit editModeChanged(editModeEnabled);
}

void AppState::setSelectedTag(TagNode* selectedTag) {
    if (m_editModeEnabled) return;
    if (m_selectedTag == selectedTag) return;
    if (m_selectedTag != nullptr && selectedTag != nullptr && *m_selectedTag == *selectedTag) return;
    m_selectedTag = selectedTag;
    emit selectedTagChanged(selectedTag);
}

bool AppState::isSelectedTagEditable()  {
    return !(m_selectedTag == nullptr || m_selectedTag->id() <= 0);
}

void AppState::toggleEditMode() {
    setEditModeEnabled(!m_editModeEnabled);
}

void AppState::signalSelectedTagUpdated() {
    emit selectedTagUpdated();
}
