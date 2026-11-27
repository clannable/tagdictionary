#include "tageditor.h"
#include "ui_tageditor.h"
#include <QFileInfo>
#include "globals.h"
#include "icondialog.h"

TagListContainer::TagListContainer(QString title, TagList* list, QWidget* parent) : QWidget(parent), m_list(list) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);
    setLayout(layout);
    layout->addWidget(new QLabel(title));
    QFrame* f = new QFrame();
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Sunken);
    layout->addWidget(list);
    layout->addWidget(f);
    connect(APP_STATE, &AppState::editModeChanged, this, [f](bool editModeEnabled) {
        if (editModeEnabled) f->hide();
        else f->show();
    });
}
//! Show/hide the horizontal line below the list widget.
void TagListContainer::setLineVisible(bool visible) {
    if (visible) layout()->itemAt(2)->widget()->show();
    else layout()->itemAt(2)->widget()->hide();
}

TagEditor::TagEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TagEditor)
{
    ui->setupUi(this);

    // Set initial view for no tag selected, edit mode disabled
    ui->iconButton->hide();
    ui->descriptionEditor->hide();
    ui->tagLabelEdit->setText("");
    ui->scrollArea->setMaximumHeight(height() * 0.7);
    ui->listContainer->hide();

    u_relatedList = new TagList();
    u_requiredList = new TagList();
    u_frequentList = new TagList();
    u_excludesList = new TagList();

    QVBoxLayout* layout = static_cast<QVBoxLayout*>(ui->listContainer->layout());
    lists.append(new TagListContainer("Required Tags", u_requiredList));
    layout->addWidget(lists[0], 1);
    lists.append(new TagListContainer("Excludes", u_excludesList));
    layout->addWidget(lists[1], 1);
    lists.append(new TagListContainer("Frequently Tagged With", u_frequentList));
    layout->addWidget(lists[2], 1);
    lists.append(new TagListContainer("Related Tags", u_relatedList));
    layout->addWidget(lists[3], 1);

    // static_cast<QVBoxLayout*>(ui->scrollArea->widget()->layout())->insertStretch(0, 1);

    // Signals, slots
    connect(ui->editButton, &QPushButton::clicked, this, [] { APP_STATE->toggleEditMode(); });
    connect(ui->saveButton, &QPushButton::clicked, this, &TagEditor::save);
    connect(ui->iconButton, &QToolButton::clicked, this, &TagEditor::selectIcon);
    connect(ui->description, &QTextBrowser::anchorClicked, this, &TagEditor::onAnchorClick);
    connect(APP_STATE, &AppState::editModeChanged, this, &TagEditor::onEditModeChange);
    connect(APP_STATE, &AppState::selectedTagChanged, this, &TagEditor::setTag);
}

TagEditor::~TagEditor()
{
    delete ui;
}

//! Set the tag shown in the editor.
//! If the selected tag is the root node, nothing will be shown and editing will be disabled.
void TagEditor::setTag(TagNode *tag) {
    if (!tag || tag->isRoot()) {
        ui->listContainer->hide();
        ui->tagLabelEdit->setText("");
        ui->editButton->setEnabled(false);
        ui->description->setMarkdown("");
        ui->description->show();
        ui->iconLabel->hide();
        ui->tagLabelEdit->setText("");
        ui->saveButton->setEnabled(false);
        return;
    }

    QString ic = QString::fromStdString(tag->icon());
    if (QFileInfo::exists(ic) || ic.startsWith(":/icons/"))
        m_iconPath = ic;
    else if (!ic.startsWith(":/icons/"))
        m_iconPath = ":/icons/" + ic;

    ui->iconLabel->show();
    ui->iconLabel->setPixmap(QIcon(m_iconPath).pixmap(QSize(20, 20)));

    ui->editButton->show();
    ui->saveButton->show();

    ui->editButton->setEnabled(true);
    ui->description->show();
    ui->description->setMarkdown(QString::fromStdString(tag->description()));
    ui->descriptionEditor->hide();
    ui->tagLabelEdit->setText(QString::fromStdString(tag->key()));

    u_relatedList->setValues(tag->related());
    u_excludesList->setValues(tag->excludes());
    u_requiredList->setValues(tag->required());
    u_frequentList->setValues(tag->frequent());

    checkEmptyLists();
}

void TagEditor::onEditModeChange(bool editModeEnabled) {
    const TagNode* selectedTag = APP_STATE->selectedTag();
    ui->tagLabelEdit->setEnabled(editModeEnabled);
    ui->saveButton->setEnabled(editModeEnabled);
    ui->editButton->setIcon(QIcon::fromTheme(editModeEnabled
        ? QIcon::ThemeIcon::EditClear
        : QIcon::ThemeIcon::MailMessageNew
    ));
    ui->editButton->setText(editModeEnabled ? "Cancel" : "Edit Tag");

    if (editModeEnabled) {
        ui->description->hide();
        ui->descriptionEditor->show();
        ui->descriptionEditor->setText(QString::fromStdString(selectedTag->description()));

        ui->iconButton->show();
        ui->iconButton->setIcon(QIcon(m_iconPath));
        ui->iconLabel->hide();

        ui->listContainer->show();
        for(const auto &l : lists) l->show();    // Always show all sub-lists when in edit mode
    }
    else {
        // Hide input fields, show read-only widgets
        ui->descriptionEditor->hide();
        ui->description->show();
        ui->description->setMarkdown(QString::fromStdString(selectedTag->description()));
        ui->tagLabelEdit->setText(QString::fromStdString(selectedTag->key()));
        ui->iconButton->hide();
        ui->iconLabel->show();
        ui->iconLabel->setPixmap(QIcon(QString::fromStdString(selectedTag->icon())).pixmap(QSize(20,20)));

        u_relatedList->setValues(selectedTag->related());
        u_excludesList->setValues(selectedTag->excludes());
        u_requiredList->setValues(selectedTag->required());
        u_frequentList->setValues(selectedTag->frequent());

        checkEmptyLists();
    }
}

//! Opens the icon selection dialog
void TagEditor::selectIcon() {
    if (!APP_STATE->isSelectedTagEditable()) return;
    IconDialog* dialog = new IconDialog(this);

    dialog->setSelected(m_iconPath);
    connect(dialog, &IconDialog::iconSelected, this, &TagEditor::iconSelected);
    dialog->exec();
}

//! Triggers when the icon selection dialog is accepted and closed.
//! Updates the icon shown in the button next to the name input field.
void TagEditor::iconSelected(QString icon) {
    if (icon == m_iconPath) return;
    if (!icon.isEmpty()) {
        m_iconPath = icon;
        QIcon ic = QIcon(m_iconPath);
        ui->iconButton->setIcon(ic);
        if (!APP_STATE->editModeEnabled()) {
            APP_STATE->selectedTag()->setIcon(m_iconPath.toStdString());
            ui->iconLabel->setPixmap(ic.pixmap(QSize(20,20)));
            APP_STATE->signalSelectedTagUpdated();
        }
    }
}

void TagEditor::cleanSublists() {
    if (!APP_STATE->selectedTag()) return;
    u_relatedList->clean();
    u_requiredList->clean();
    u_frequentList->clean();
    u_excludesList->clean();
}

/*!
 *  Updates the following values in the currently selected tag with their related controls:
*   - name
*   - description
*   - icon
*   - sub-lists
*   - if the "replaces parent" option is checked
*
* Then emits the `tagSaved` signal to the main window so that the tag's file list can be updated.
*/
void TagEditor::save() {
    TagNode* tag = APP_STATE->selectedTag();

    tag->setKey(ui->tagLabelEdit->text());
    tag->setDescription(ui->descriptionEditor->toPlainText());
    tag->setIcon(m_iconPath);
    tag->setRelated(u_relatedList->values());
    tag->setRequired(u_requiredList->values());
    tag->setFrequent(u_frequentList->values());
    tag->setExcludes(u_excludesList->values());
    tag->setReplacesParent(ui->replaceFrame->isReplaceChecked());

    emit tagSaved(tag);
}

/*!
 *  Triggers when an internal link starting with `"#"` is clicked in the tag description.
 * If the link starts with `"#$"`, emits the `displayLinkClicked` signal containing the page number the MediaDisplay widget should navigate to.
 * Otherwise, it is treated link a normal anchor link in the description viewer.
 */
void TagEditor::onAnchorClick(const QUrl &link) {
    QString path = link.toString();
    if (path.startsWith("#$"))
        emit displayLinkClicked(path.slice(2).toInt()-1);
    else
        ui->description->scrollToAnchor(path.slice(1));
}

void TagEditor::resizeEvent(QResizeEvent* event) {
    ui->scrollArea->setMaximumHeight(height() * 0.7);
}

void TagEditor::checkEmptyLists() {
    if (HIDE_EMPTY_LISTS) {
        // If HIDE_EMPTY_LISTS is enabled, do not show any empty-sublists.
        int lastIndex = -1; // The index of the bottom-most visible list
        for (int i = 0; i < lists.count(); i++) {
            if (lists[i]->empty()) lists[i]->hide();
            else { lists[i]->show(); lastIndex = max(i, lastIndex); }
        }
        if (lastIndex == -1) ui->listContainer->hide(); // If no lists are visible, hide the containing widget.
        else {
            ui->listContainer->show();
            // Make the separating horizontal line in each list container widget visible except for the bottom-most visible list.
            // This effectively collapses the line of the last list into the bottom border of the scroll area.
            for (int j = 0; j < lists.count(); j++)
                lists[j]->setLineVisible(true);
            lists[lastIndex]->setLineVisible(false);
        }
    } else {
        ui->listContainer->show();
        // If not enabled, show all lists and make all horizontal separators visible except the last one.
        for (int k = 0; k < lists.count(); k++) {
            lists[k]->show();
            lists[k]->setLineVisible(true);
        }
        lists[lists.count()-1]->setLineVisible(false);
    }
}
