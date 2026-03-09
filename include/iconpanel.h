#ifndef ICONPANEL_H
#define ICONPANEL_H

#include <QWidget>

namespace Ui {
class IconPanel;
}

class IconPanel : public QWidget
{
    Q_OBJECT

public:
    explicit IconPanel(QString iconPath, QWidget *parent = nullptr);
    ~IconPanel();

    void setSelected(bool selected);
    QString getIcon();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
signals:
    void selected(IconPanel *panel);

private:
    Ui::IconPanel *ui;
    QString icon;
    bool isSelected;
    QString baseStyleSheet;
};

#endif // ICONPANEL_H
