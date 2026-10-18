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
    QString icon() const { return m_icon; }
    QString shortName() const { return m_shortName; }

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

signals:
    void clicked(IconPanel *panel);

private:
    Ui::IconPanel *ui;
    QString m_icon;
    QString m_shortName;
    bool m_selected = false;
};

#endif // ICONPANEL_H
