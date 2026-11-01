#ifndef NEWTAGDIALOG_H
#define NEWTAGDIALOG_H
#include <QDialog>
#include "tagnode.h"
#include "icondialog.h"

namespace Ui {
class NewTagDialog;
}

class NewTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTagDialog(QWidget *parent = nullptr);
    ~NewTagDialog();

    void setTag(int id);

signals:
    void submit(TagNode* node);

protected slots:
    void onNameChange(QString text);
    virtual void accept() override;
    void selectIcon();
    void updateIcon(QString iconPath);
    void resetIcon();

private:
    Ui::NewTagDialog *ui;
    QString m_icon;
    QStringList m_invalidNames;
};

#endif // NEWTAGDIALOG_H
