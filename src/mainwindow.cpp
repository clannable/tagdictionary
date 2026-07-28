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
bool AUTOSAVE_ENABLED = true;
const int MAX_RECENT = 5;


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

    openAction = new QAction("Open Dictionary...", this);
    recentMenu = new QMenu("Open Recent File", this);
    saveAction = new QAction("Save Dictionary", this);

    newAction->setShortcuts(QKeySequence::New);
    openAction->setShortcuts(QKeySequence::Open);
    saveAction->setShortcuts(QKeySequence::Save);
    QAction *autoSaveAction = new QAction("Automatically Save Changes", this);
    autoSaveAction->setCheckable(true);
    AUTOSAVE_ENABLED = settings.value("autosave", true).toBool();
    autoSaveAction->setChecked(AUTOSAVE_ENABLED);
    autoSaveAction->setToolTip("Automatically save changes to the current dictionary file whenever tags are updated or files are dropped in.");
    connect(autoSaveAction, &QAction::toggled, this, &MainWindow::onToggleAutoSave);

    connect(newAction, &QAction::triggered, this, &MainWindow::newJson);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveJson);
    connect(openAction, &QAction::triggered, this, &MainWindow::openJson);
    fileMenu->addAction(newAction);
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    fileMenu->addMenu(recentMenu);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAction);
    fileMenu->addAction(autoSaveAction);
    setupRecentFileList();

    QMenu *tagMenu = ui->menuBar->addMenu("Tags");
    newTagAction = new QAction("Create New Tag", this);
    iconAction = new QAction("Modify Icon", this);
    iconAction->setShortcut(QKeySequence(QKeyCombination(Qt::AltModifier, Qt::Key_I)));
    connect(newTagAction, &QAction::triggered, ui->tagTree, &TagTree::onCreateTag);
    connect(iconAction, &QAction::triggered, ui->tagEditor, &TagEditor::selectIcon);
    iconAction->setDisabled(true);

    tagMenu->addAction(newTagAction);
    tagMenu->addSeparator();
    tagMenu->addAction(iconAction);


    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->tagTree, &TagTree::setEditMode);
    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->mediaDisplay, &MediaDisplay::setEditMode);
    connect(ui->tagEditor, &TagEditor::listItemSelected, this, &MainWindow::onTagListSelect);
    connect(ui->tagEditor, &TagEditor::tagSaved, this, &MainWindow::onSave);
    connect(ui->tagEditor, &TagEditor::partialSave, this, &MainWindow::onPartialSave);
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
    if (editModeEnabled || ui->tagTree->selectedItems().isEmpty()) {
        iconAction->setDisabled(true);
        return;
    }
    iconAction->setDisabled(false);
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

    if (AUTOSAVE_ENABLED)
        saveJson();
}

void MainWindow::onPartialSave(TagNode *tag) {
    selectedItem->setText(0, QString::fromStdString(tag->getKey()));
    selectedItem->setIcon(0, QIcon(QString::fromStdString(tag->getIcon())));

    if (AUTOSAVE_ENABLED)
        saveJson();
}

/*--------- Media Display Slots ---------*/

void MainWindow::onAddFile(QString filePath) {
    if (editModeEnabled) return; // Don't update json data if tag is still being edited
    selectedItem->getNode()->addFile(filePath.toStdString());
    selectedItem->refreshFileIcons();
    if (AUTOSAVE_ENABLED)
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
            saveAction->setEnabled(false);
        } else {
            of << json({}).dump(2);
            of.close();
            QSettings settings("MyApp","Tag Viewer");
            settings.setValue("data/lastOpened", jsonFilePath);
            pushToRecent();
            reloadJson();
            saveAction->setEnabled(!jsonFilePath.isEmpty());
        }
    }
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
    pushToRecent();
    reloadJson();
}

void MainWindow::reloadJson() {
    if (jsonFilePath.isNull() || jsonFilePath.isEmpty()) return;
    ui->tagTree->clear();
    editModeEnabled = false;

    selectedItem = nullptr;

    ui->mediaDisplay->setFilesFromNode(nullptr);
    ui->tagEditor->setTag(nullptr);
    std::ifstream* f;

    try {
        f = new std::ifstream(jsonFilePath.toStdString());
        ICON_LIST->clear();

        QDirIterator it(":/icons/", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            if (path.lastIndexOf(".") == -1) continue;
            ICON_LIST->push_back(path.toStdString());
        }

        json tags = json::parse(*f);
        ui->tagTree->fromJson(tags);
        ui->tagTree->sortByColumn(0, Qt::AscendingOrder);
    } catch (std::exception e) {
        QMessageBox::critical(this, "An error has occurred", "Error loading dictionary: \n" + QString::fromStdString(e.what()));
        ui->tagTree->clear();
    }

    if (f->is_open())
        f->close();
}

void MainWindow::onToggleAutoSave(bool checked) {
    AUTOSAVE_ENABLED = checked;
    QSettings settings("MyApp", "Tag Viewer");
    settings.setValue("autosave", AUTOSAVE_ENABLED);
}

void MainWindow::pushToRecent() {
    QSettings settings("MyApp", "Tag Viewer");
    QStringList recentFiles = settings.value("recentFiles").value<QStringList>();

    if (recentFiles.contains(jsonFilePath)) {
        int index = recentFiles.indexOf(jsonFilePath);
        recentFiles.removeAt(index);
    } else if (recentFiles.size() == MAX_RECENT) {
        recentFiles.removeLast();
    }

    recentFiles.push_front(jsonFilePath);
    settings.setValue("recentFiles", recentFiles);

    setupRecentFileList();
}

void MainWindow::setupRecentFileList() {
    recentMenu->clear();
    QSettings settings("MyApp", "Tag Viewer");
    QStringList recentFiles = settings.value("recentFiles").value<QStringList>();

    recentMenu->menuAction()->setEnabled(!recentFiles.isEmpty());

    for (int i = 0; i < recentFiles.length(); i++) {
        QString filePath = recentFiles.at(i);
        QString label = QString::number(i+1) + ". \t\t" + QFileInfo(filePath).fileName();
        QAction* fileAction = new QAction(label, this);
        fileAction->setToolTip(jsonFilePath);
        recentMenu->addAction(fileAction);
        connect(fileAction, &QAction::triggered, this, [filePath, this]() {
            std::cout << "Triggered recent file action for " << filePath.toStdString() << "\n" << std::flush;
            this->jsonFilePath = filePath;
            this->pushToRecent();
            this->reloadJson();
        });
    }
}
