#include "tageditor.h"
#include "ui_tageditor.h"
#include <QFileInfo>
#include "globals.h"
#include "icondialog.h"

TagEditor::TagEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TagEditor)
{
    ui->setupUi(this);
    ui->iconButton->hide();
    ui->descriptionEditor->hide();
    ui->tagLabelEdit->setText("");


    u_relatedList = new TagList();
    u_requiredList = new TagList();
    u_frequentList = new TagList();

    ui->listContainer->layout()->addWidget(createHLine());
    ui->listContainer->layout()->addWidget(new QLabel("Required Tags"));
    ui->listContainer->layout()->addWidget(u_requiredList);
    ui->listContainer->layout()->addWidget(createHLine());
    ui->listContainer->layout()->addWidget(new QLabel("Frequently Tagged With"));
    ui->listContainer->layout()->addWidget(u_frequentList);
    ui->listContainer->layout()->addWidget(createHLine());
    ui->listContainer->layout()->addWidget(new QLabel("Related Tags"));
    ui->listContainer->layout()->addWidget(u_relatedList);

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

void TagEditor::setTag(TagNode *tag) {

    if (tag == nullptr) {
        ui->listContainer->hide();
        ui->editButton->setEnabled(false);
        ui->tagLabelEdit->setText("");
        ui->iconButton->setIcon(QIcon());
        ui->iconLabel->setPixmap(QPixmap());
        ui->description->setMarkdown("");
        ui->description->show();
        return;
    }
    else if (tag->isRoot()) {
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
    ui->iconLabel->show();
    ui->editButton->show();
    ui->saveButton->show();
    u_relatedList->setValues(tag->related());
    u_requiredList->setValues(tag->required());
    u_frequentList->setValues(tag->frequent());

    ui->listContainer->show();
    ui->editButton->setEnabled(true);
    ui->description->show();
    ui->descriptionEditor->hide();

    QString ic = QString::fromStdString(tag->icon());
    if (QFileInfo::exists(ic) || ic.startsWith(":/icons/"))
        m_iconPath = ic;
    else if (!ic.startsWith(":/icons/"))
        m_iconPath = ":/icons/" + ic;

    QIcon icon = QIcon(m_iconPath);
    QString description = QString::fromStdString(tag->description());

    ui->iconLabel->setPixmap(icon.pixmap(QSize(20, 20)));
    ui->description->setMarkdown(description);
    ui->descriptionEditor->setText(description);
    ui->tagLabelEdit->setText(QString::fromStdString(tag->key()));
    ui->iconButton->setIcon(QIcon(m_iconPath));
}

void TagEditor::onEditModeChange(bool editModeEnabled) {
    ui->tagLabelEdit->setEnabled(editModeEnabled);
    ui->saveButton->setEnabled(editModeEnabled);
    ui->editButton->setIcon(QIcon::fromTheme(editModeEnabled
        ? QIcon::ThemeIcon::EditClear
        : QIcon::ThemeIcon::MailMessageNew
    ));
    ui->editButton->setText(editModeEnabled ? "Cancel" : "Edit Tag");
    if (editModeEnabled) {
        ui->description->hide();
        ui->descriptionEditor->setText(QString::fromStdString(APP_STATE->selectedTag()->description()));
        ui->descriptionEditor->show();
        ui->iconButton->show();
        ui->iconLabel->hide();
    }
    else {
        ui->descriptionEditor->hide();
        ui->description->show();
        ui->iconButton->hide();
        ui->iconLabel->show();
        u_relatedList->setValues(APP_STATE->selectedTag()->related());
        u_requiredList->setValues(APP_STATE->selectedTag()->required());
        u_frequentList->setValues(APP_STATE->selectedTag()->frequent());
    }
}

void TagEditor::selectIcon() {
    if (!APP_STATE->isSelectedTagEditable()) return;
    IconDialog* dialog = new IconDialog(this);

    dialog->setSelected(m_iconPath);
    connect(dialog, &IconDialog::iconSelected, this, &TagEditor::iconSelected);
    dialog->exec();
}

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

void TagEditor::save() {
    QIcon icon = QIcon(m_iconPath);
    ui->iconLabel->setPixmap(icon.pixmap(QSize(20,20)));
    QString updatedDescription = ui->descriptionEditor->toPlainText();
    ui->description->setMarkdown(updatedDescription);

    TagNode* tag = APP_STATE->selectedTag();

    tag->setKey(ui->tagLabelEdit->text());
    tag->setDescription(updatedDescription);
    tag->setIcon(m_iconPath);
    tag->setRelated(json(u_relatedList->values()));
    tag->setRequired(json(u_requiredList->values()));
    tag->setFrequent(json(u_frequentList->values()));

    emit tagSaved(tag);
}

void TagEditor::onAnchorClick(const QUrl &link) {
    QString path = link.toString();
    if (path.startsWith("#$"))
        emit displayLinkClicked(path.slice(2).toInt()-1);
    else
        ui->description->scrollToAnchor(path.slice(1));
}

QFrame* TagEditor::createHLine() {
    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);
    return frame;
}
