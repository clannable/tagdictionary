#ifndef NEWTAGDIALOG_H
#define NEWTAGDIALOG_H
#include <QDialog>
#include "./tagnode.h"
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

    void setTag(TagNode* node);

public slots:
    void onNameChange(QString text);
    virtual void accept() override;
    void onIconSelect();
    void onIconUpdate(QString iconPath);
    void resetIcon();

signals:
    void submit(TagNode* node);


private:
    Ui::NewTagDialog *ui;
    IconDialog* iconDialog;
    QString iconPath;
    QStringList invalidNames;
};

#endif // NEWTAGDIALOG_H
