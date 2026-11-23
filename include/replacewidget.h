#ifndef REPLACEWIDGET_H
#define REPLACEWIDGET_H

#include "tagnode.h"
#include <QWidget>


class ReplaceWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ReplaceWidget(QWidget *parent = nullptr);

    bool isReplaceChecked();
    void setReplaceChecked(bool replace);

public slots:
    void onEditModeChange(bool editModeEnabled);
    void setTag(const TagNode* tag);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
};

#endif // REPLACEWIDGET_H
