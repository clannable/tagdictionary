#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jsonnode.h"
#include "tagtreeitem.h"

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


using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tagEditor->linkTagTreeToLists(ui->tagTree);
    QMenu *fileMenu = ui->menuBar->addMenu("File");
    QAction *saveAction = new QAction("Save JSON", this);
    QAction *openAction = new QAction("Open...", this);
    openAction->setShortcuts(QKeySequence::Open);
    saveAction->setShortcuts(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveJson);
    connect(openAction, &QAction::triggered, this, &MainWindow::openJson);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(openAction);

    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->tagTree, &TagTree::setEditMode);
    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->mediaDisplay, &MediaDisplay::setEditMode);
    connect(ui->tagEditor, &TagEditor::listItemSelected, this, &MainWindow::onTagListSelect);
    connect(ui->tagEditor, &TagEditor::tagSaved, this, &MainWindow::onSave);
    connect(ui->tagEditor, &TagEditor::editModeChanged, this, &MainWindow::setEditMode);
    connect(ui->mediaDisplay, &MediaDisplay::fileAdded, this, &MainWindow::onAddFile);

    connect(ui->tagTree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onTagDoubleClicked);
    connect(ui->tagTree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onTagSelect);
    connect(ui->tagTree, &TagTree::tagsChanged, this, &MainWindow::saveJson);
    connect(ui->tagTree, &TagTree::addToRelated, ui->tagEditor, &TagEditor::addToRelated);
    connect(ui->tagTree, &TagTree::addToRequired, ui->tagEditor, &TagEditor::addToRequired);

    QSettings settings("MyApp","Tag Viewer");
    jsonFilePath = settings.value("data/lastOpened", "").toString();

    reloadJson();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*---------  Tag Tree Slots ---------*/

void MainWindow::onTagSelect() {
    if (editModeEnabled) return;
    selectedItem = static_cast<TagTreeItem*>(ui->tagTree->currentItem());

    JsonNode *node = selectedItem->getNode();
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

    TagTreeItem *tag = ui->tagTree->findTag(tagPath);
    selectedItem->setSelected(false);
    selectedItem = tag;
    selectedItem->setSelected(true);
    ui->tagTree->expandItem(selectedItem);
}

/*--------- Tag Editor Slots ---------*/

void MainWindow::setEditMode(bool mode) {
    editModeEnabled = mode;

    QFont font = selectedItem->font(0);
    font.setWeight(mode ? QFont::DemiBold : QFont::Normal);
    selectedItem->setFont(0, font);

    ui->tagTree->setCurrentItem(selectedItem);
}

void MainWindow::onSave(json tag) {

    QString key = QString::fromStdString(tag.value("key", ""));
    tag.erase("key");
    ui->mediaDisplay->save();
    QStringList files = ui->mediaDisplay->getFiles();
    std::list<std::string> fileList;
    for (const QString& f : files)
        fileList.push_back(f.toStdString());

    tag["files"] = json(fileList);
    selectedItem->setKey(key);
    selectedItem->getNode()->setData(tag);
    selectedItem->setIcon(0, QIcon(QString::fromStdString(tag.value("icon", ""))));


    ui->mediaDisplay->setFilesFromNode(selectedItem->getNode());
    saveJson();
}

/*--------- Media Display Slots ---------*/

void MainWindow::onAddFile(QString filePath) {
    if (editModeEnabled) return; // Don't update json data if tag is still being edited
    json data = selectedItem->getJson();
    data.emplace("files", json::array());
    data["files"].push_back(filePath.toStdString());
    selectedItem->getNode()->setData(data);
    saveJson();
}


/*--------- JSON Functions ---------*/

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
        "/home",
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

    std::cout << "Loading JSON file " << jsonFilePath.toStdString() << "\n" << std::flush;

    std::ifstream f(jsonFilePath.toStdString());
    json tags = json::parse(f);
    ui->tagTree->fromJson(tags);
    ui->tagTree->sortByColumn(0, Qt::AscendingOrder);

    f.close();
}
