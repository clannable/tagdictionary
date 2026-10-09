#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tagnode.h"
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
std::string LAST_ICON_FOLDER_PATH = "/home";
int NEXT_TAG_ID = 1;
std::map<int, TagNode*> TAGS = {};
std::map<std::string, int> TAG_PATH_MAP = {};
std::chrono::milliseconds DEBOUNCE_TIME = 250ms;
bool AUTOSAVE_ENABLED = true;
const int MAX_RECENT = 5;
AppState* APP_STATE = new AppState();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings settings("MyApp","Tag Viewer");
    if (settings.contains("geometry"))
        this->restoreGeometry(settings.value("geometry").toByteArray());

    // ui->tagEditor->linkTagTreeToLists(ui->tagTree);
    QMenu* fileMenu = ui->menuBar->addMenu("File");
    QAction* newAction = new QAction("New Dictionary", this);

    a_open = new QAction("Open Dictionary...", this);
    a_save = new QAction("Save Dictionary", this);
    u_recent = new QMenu("Open Recent File", this);

    newAction->setShortcuts(QKeySequence::New);
    a_open->setShortcuts(QKeySequence::Open);
    a_save->setShortcuts(QKeySequence::Save);

    QAction *autoSaveAction = new QAction("Automatically Save Changes", this);
    autoSaveAction->setCheckable(true);

    AUTOSAVE_ENABLED = settings.value("autosave", true).toBool();
    autoSaveAction->setChecked(AUTOSAVE_ENABLED);
    autoSaveAction->setToolTip("Automatically save changes to the current dictionary file whenever tags are updated or files are dropped in.");

    connect(autoSaveAction, &QAction::toggled, this, &MainWindow::onToggleAutoSave);

    connect(newAction, &QAction::triggered, this, &MainWindow::newJson);
    connect(a_save, &QAction::triggered, this, &MainWindow::saveJson);
    connect(a_open, &QAction::triggered, this, &MainWindow::openJson);
    fileMenu->addAction(newAction);
    fileMenu->addSeparator();
    fileMenu->addAction(a_open);
    fileMenu->addMenu(u_recent);
    fileMenu->addSeparator();
    fileMenu->addAction(a_save);
    fileMenu->addAction(autoSaveAction);
    setupRecentFileList();

    QMenu *tagMenu = ui->menuBar->addMenu("Tags");
    a_newTag = new QAction("Create New Tag", this);
    a_icon = new QAction("Modify Icon", this);
    a_icon->setShortcut(QKeySequence(QKeyCombination(Qt::AltModifier, Qt::Key_I)));
    connect(a_newTag, &QAction::triggered, ui->tagTree, [this] { ui->tagTree->onCreateTag(nullptr); });
    connect(a_icon, &QAction::triggered, ui->tagEditor, &TagEditor::selectIcon);
    a_icon->setDisabled(true);

    tagMenu->addAction(a_newTag);
    tagMenu->addSeparator();
    tagMenu->addAction(a_icon);

    connect(ui->tagEditor, &TagEditor::tagSaved, this, &MainWindow::onEditorSave);
    connect(APP_STATE, &AppState::selectedTagUpdated, this, &MainWindow::onTagUpdate);
    connect(ui->tagEditor, &TagEditor::displayLinkClicked, ui->mediaDisplay, &MediaDisplay::setFile);

    m_searchDebounce = new QTimer(this);
    m_searchDebounce->setSingleShot(true);

    connect(ui->searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchChange);
    connect(m_searchDebounce, &QTimer::timeout, this, &MainWindow::onSearchTimeout);

    connect(ui->tagTree, &TagTree::tagsChanged, this, &MainWindow::onTagTreeChange);
    // connect(ui->tagTree, &TagTree::addToRelated, ui->tagEditor, &TagEditor::addToRelated);
    // connect(ui->tagTree, &TagTree::addToRequired, ui->tagEditor, &TagEditor::addToRequired);

    m_jsonFilePath = settings.value("data/lastOpened", "").toString();
    if (!m_jsonFilePath.isEmpty() && !QFileInfo::exists(m_jsonFilePath)) {
        QMessageBox::critical(this, "Failed to load file", "ERROR: Failed to load dictionary file: \n\tFile not found");
        m_jsonFilePath = "";
    }
    a_save->setEnabled(!m_jsonFilePath.isEmpty());
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

void MainWindow::onTagTreeChange() {
    if (AUTOSAVE_ENABLED)
        saveJson();
}

void MainWindow::onSearchChange() {
    m_searchDebounce->start(DEBOUNCE_TIME);
}

void MainWindow::onSearchTimeout() {
    ui->tagTree->filterTree(ui->searchInput->text());
}

/*--------- Tag Editor Slots ---------*/

void MainWindow::onEditorSave() {
    ui->mediaDisplay->save();
    QStringList files = ui->mediaDisplay->files();
    APP_STATE->selectedTag()->setFiles(files);

    ui->tagTree->sortItems(0, Qt::AscendingOrder);
    // ui->mediaDisplay->setFilesFromNode(tag);
    ui->tagTree->setCurrentItem(APP_STATE->selectedTag()->leaf());
    APP_STATE->signalSelectedTagUpdated();
    APP_STATE->setEditModeEnabled(false);
}

void MainWindow::onTagUpdate() {
    if (AUTOSAVE_ENABLED)
        saveJson();
}

/*--------- JSON Functions ---------*/

void MainWindow::newJson() {
    m_jsonFilePath = QFileDialog::getSaveFileName(
        this,
        "Select where to save JSON file",
        m_jsonFilePath.isEmpty() ? "" : QFileInfo(m_jsonFilePath).dir().absolutePath(),
        "Tag Dictionary (*.json)"
    );
    if (!m_jsonFilePath.isEmpty()) {
        std::ofstream of(m_jsonFilePath.toStdString());
        if (!of.is_open()) {
            // std::cout << "Failed to open output file\n" << std::flush;
            a_save->setEnabled(false);
        } else {
            of << json({}).dump(2);
            of.close();
            QSettings settings("MyApp","Tag Viewer");
            settings.setValue("data/lastOpened", m_jsonFilePath);
            pushToRecent();
            reloadJson();
            a_save->setEnabled(!m_jsonFilePath.isEmpty());
        }
    }
}

void MainWindow::saveJson() {
    if (m_jsonFilePath.isNull() || m_jsonFilePath.isEmpty()) return;
    json tags = ui->tagTree->toJson();
    std::ofstream of(m_jsonFilePath.toStdString());
    if (!of.is_open()) {
        std::cout << "Failed to open output file\n" << std::flush;
    } else {
        of << tags.dump(2);
        of.close();
    }
}

void MainWindow::openJson() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select JSON file",
        m_jsonFilePath.isEmpty() ? "" : QFileInfo(m_jsonFilePath).dir().absolutePath(),
        "Tag Dictionary (*.json)");
    if (!filePath.isEmpty()) {
        m_jsonFilePath = filePath;
        QSettings settings("MyApp","Tag Viewer");
        settings.setValue("data/lastOpened", m_jsonFilePath);

        pushToRecent();
        reloadJson();
    }
}

void MainWindow::reloadJson() {
    if (m_jsonFilePath.isNull() || m_jsonFilePath.isEmpty()) return;
    APP_STATE->setEditModeEnabled(false);
    APP_STATE->setSelectedTag(nullptr);
    ui->tagTree->clear();
    std::ifstream* f;

    try {
        f = new std::ifstream(m_jsonFilePath.toStdString());
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
        QMessageBox::critical(this, "An error has occurred", "Error loading dictionary");
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

    if (recentFiles.contains(m_jsonFilePath)) {
        int index = recentFiles.indexOf(m_jsonFilePath);
        recentFiles.removeAt(index);
    } else if (recentFiles.size() == MAX_RECENT) {
        recentFiles.removeLast();
    }

    recentFiles.push_front(m_jsonFilePath);
    settings.setValue("recentFiles", recentFiles);

    setupRecentFileList();
}

void MainWindow::setupRecentFileList() {
    u_recent->clear();
    QSettings settings("MyApp", "Tag Viewer");
    QStringList recentFiles = settings.value("recentFiles").value<QStringList>();

    u_recent->menuAction()->setEnabled(!recentFiles.isEmpty());

    for (int i = 0; i < recentFiles.length(); i++) {
        QString filePath = recentFiles.at(i);
        QString label = QString::number(i+1) + ". \t\t" + QFileInfo(filePath).fileName();
        QAction* fileAction = new QAction(label, this);
        fileAction->setToolTip(m_jsonFilePath);
        u_recent->addAction(fileAction);
        connect(fileAction, &QAction::triggered, this, [filePath, this]() {
            m_jsonFilePath = filePath;
            QSettings settings("MyApp","Tag Viewer");
            settings.setValue("data/lastOpened", m_jsonFilePath);
            this->pushToRecent();
            this->reloadJson();
        });
    }
}
