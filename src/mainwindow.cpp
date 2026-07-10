#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tagnode.h"
#include "tagtreeitem.h"
#include "globals.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <QMenuBar>
#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QMimeDatabase>
#include <QAudioOutput>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDirIterator>
#include <QLineEdit>

using json = nlohmann::json;

std::list<std::string>* ICON_LIST = new std::list<std::string>();
std::string LAST_IMAGE_FOLDER_PATH = "/home";
std::chrono::milliseconds DEBOUNCE_TIME = 250ms;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings settings("MyApp","Tag Viewer");
    if (settings.contains("geometry"))
        this->restoreGeometry(settings.value("geometry").toByteArray());

    ui->tagEditor->linkTagTreeToLists(ui->tagTree);
    QMenu *fileMenu = ui->menuBar->addMenu("File");
    QAction *newAction = new QAction("New Dictionary", this);
    saveAction = new QAction("Save Dictionary", this);
    openAction = new QAction("Open Dictionary...", this);
    newAction->setShortcuts(QKeySequence::New);
    openAction->setShortcuts(QKeySequence::Open);
    saveAction->setShortcuts(QKeySequence::Save);
    connect(newAction, &QAction::triggered, this, &MainWindow::newJson);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveJson);
    connect(openAction, &QAction::triggered, this, &MainWindow::openJson);
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);

    QMenu *tagMenu = ui->menuBar->addMenu("Tags");
    newTagAction = new QAction("Create New Tag", this);

    connect(newTagAction, &QAction::triggered, ui->tagTree, &TagTree::onCreateTag);
    tagMenu->addAction(newTagAction);

    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->tagTree, &TagTree::setEditMode);
    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->mediaDisplay, &MediaDisplay::setEditMode);
    connect(ui->tagEditor, &TagEditor::listItemSelected, this, &MainWindow::onTagListSelect);
    connect(ui->tagEditor, &TagEditor::tagSaved, this, &MainWindow::onSave);
    connect(ui->tagEditor, &TagEditor::editModeChanged, this, &MainWindow::setEditMode);
    connect(ui->mediaDisplay, &MediaDisplay::fileAdded, this, &MainWindow::onAddFile);

    searchDebounce = new QTimer(this);
    searchDebounce->setSingleShot(true);

    connect(ui->searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchChange);
    connect(searchDebounce, &QTimer::timeout, this, &MainWindow::onSearchTimeout);

    connect(ui->tagTree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onTagDoubleClicked);
    connect(ui->tagTree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onTagSelect);
    connect(ui->tagTree, &TagTree::tagsChanged, this, &MainWindow::saveJson);
    connect(ui->tagTree, &TagTree::addToRelated, ui->tagEditor, &TagEditor::addToRelated);
    connect(ui->tagTree, &TagTree::addToRequired, ui->tagEditor, &TagEditor::addToRequired);
    connect(ui->tagTree, &TagTree::tagsChanged, ui->tagEditor, &TagEditor::refreshLists);
    jsonFilePath = settings.value("data/lastOpened", "").toString();
    if (!jsonFilePath.isEmpty() && !QFileInfo::exists(jsonFilePath)) {
        QMessageBox::critical(this, "Failed to load file", "ERROR: Failed to load dictionary file");
        jsonFilePath = "";
    }
    saveAction->setEnabled(!jsonFilePath.isEmpty());
    reloadJson();
}

MainWindow::~MainWindow()
{
    QSettings settings("MyApp","Tag Viewer");
    settings.setValue("geometry", this->saveGeometry());
    delete ui;
    delete ICON_LIST;
}

/*---------  Tag Tree Slots ---------*/

void MainWindow::onTagSelect() {
    if (editModeEnabled || ui->tagTree->selectedItems().isEmpty()) return;
    selectedItem = static_cast<TagTreeItem*>(ui->tagTree->selectedItems().first());

    TagNode *node = selectedItem->getNode();
    ui->tagEditor->setTag(node);
    ui->mediaDisplay->setFilesFromNode(node);
}

void MainWindow::onTagDoubleClicked(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column);
    if (editModeEnabled) return;
    if (item != selectedItem)
        selectedItem = static_cast<TagTreeItem*>(item);

    ui->tagEditor->toggleEditMode();
}

void MainWindow::onTagListSelect(QString tagPath) {
    if (editModeEnabled == true) return;

    TagTreeItem *tag = static_cast<TagTreeItem*>(ui->tagTree->findTag(tagPath));
    selectedItem->setSelected(false);
    selectedItem = tag;
    tag->setSelected(true);
    ui->tagTree->expandTreeTo(tag);
}

void MainWindow::onSearchChange() {
    searchDebounce->start(DEBOUNCE_TIME);
}

void MainWindow::onSearchTimeout() {
    ui->tagTree->filterTree(ui->searchInput->text());
}

/*--------- Tag Editor Slots ---------*/

void MainWindow::setEditMode(bool mode) {
    editModeEnabled = mode;

    QFont font = selectedItem->font(0);
    font.setWeight(mode ? QFont::DemiBold : QFont::Normal);
    selectedItem->setFont(0, font);

    ui->tagTree->setCurrentItem(selectedItem);
}

void MainWindow::onSave(TagNode* tag, std::string oldPath) {

    ui->mediaDisplay->save();
    QStringList files = ui->mediaDisplay->getFiles();
    std::list<std::string> fileList;
    for (const auto& f : files)
        fileList.push_back(f.toStdString());
    tag->setFiles(fileList);

    selectedItem->setText(0, QString::fromStdString(tag->getKey()));
    selectedItem->setIcon(0, QIcon(QString::fromStdString(tag->getIcon())));
    selectedItem->refreshFileIcons();

    ui->mediaDisplay->setFilesFromNode(selectedItem->getNode());
    saveJson();
}

/*--------- Media Display Slots ---------*/

void MainWindow::onAddFile(QString filePath) {
    if (editModeEnabled) return; // Don't update json data if tag is still being edited
    selectedItem->getNode()->addFile(filePath.toStdString());
    selectedItem->refreshFileIcons();
    saveJson();
}


/*--------- JSON Functions ---------*/

void MainWindow::newJson() {
    jsonFilePath = QFileDialog::getSaveFileName(
        this,
        "Select where to save JSON file",
        jsonFilePath.isEmpty() ? "" : QFileInfo(jsonFilePath).dir().absolutePath(),
        "Tag Dictionary (*.json)"
    );
    if (!jsonFilePath.isEmpty()) {
        std::ofstream of(jsonFilePath.toStdString());
        if (!of.is_open()) {
            std::cout << "Failed to open output file\n" << std::flush;
        } else {
            of << json({}).dump(2);
            of.close();
        }
        QSettings settings("MyApp","Tag Viewer");
        settings.setValue("data/lastOpened", jsonFilePath);
        reloadJson();
    }

    saveAction->setEnabled(!jsonFilePath.isEmpty());


}

void MainWindow::saveJson() {
    if (jsonFilePath.isNull() || jsonFilePath.isEmpty()) return;
    json tags = ui->tagTree->toJson();
    std::ofstream of(jsonFilePath.toStdString());
    if (!of.is_open()) {
        std::cout << "Failed to open output file\n" << std::flush;
    } else {
        of << tags.dump(2);
        of.close();
    }
}

void MainWindow::openJson() {
    jsonFilePath = QFileDialog::getOpenFileName(
        this,
        "Select JSON file",
        jsonFilePath.isEmpty() ? "" : QFileInfo(jsonFilePath).dir().absolutePath(),
        "Tag Dictionary (*.json)");
    QSettings settings("MyApp","Tag Viewer");
    settings.setValue("data/lastOpened", jsonFilePath);
    reloadJson();
}

void MainWindow::reloadJson() {
    if (jsonFilePath.isNull() || jsonFilePath.isEmpty()) return;
    ui->tagTree->clear();
    editModeEnabled = false;

    selectedItem = nullptr;

    ui->mediaDisplay->setFilesFromNode(nullptr);
    ui->tagEditor->setTag(nullptr);

    std::ifstream f(jsonFilePath.toStdString());
    ICON_LIST->clear();

    QDirIterator it(":/icons/", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (path.lastIndexOf(".") == -1) continue;
        ICON_LIST->push_back(path.toStdString());
    }

    json tags = json::parse(f);
    ui->tagTree->fromJson(tags);
    ui->tagTree->sortByColumn(0, Qt::AscendingOrder);

    f.close();
}
