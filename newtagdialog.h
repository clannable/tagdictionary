#ifndef NEWTAGDIALOG_H
#define NEWTAGDIALOG_H

#include <QDialog>
#include "./jsonnode.h"
#include "./icondialog.h"
namespace Ui {
class NewTagDialog;
}

class NewTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTagDialog(QWidget *parent = nullptr);
    ~NewTagDialog();

    void setParentNode(JsonNode* node);

public slots:
    void onNameChange(QString text);
    virtual void accept() override;
    void onIconSelect();
    void onIconUpdate(QString iconPath);
    void resetIcon();

signals:
    void submit(JsonNode* node);


private:
    Ui::NewTagDialog *ui;
    JsonNode* parentNode;
    IconDialog* iconDialog;
    QString iconPath;
};

#endif // NEWTAGDIALOG_H
