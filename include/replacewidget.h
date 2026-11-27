#ifndef REPLACEWIDGET_H
#define REPLACEWIDGET_H

#include "tagnode.h"
#include <QWidget>
#include <QLabel>
#include <QComboBox>

class ReplaceWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ReplaceWidget(QWidget *parent = nullptr);

    TagNode::ParentLink value();
    void setValue(TagNode::ParentLink val);

public slots:
    void onEditModeChange(bool editModeEnabled);
    void setTag(TagNode* tag);

private:
    QLabel* u_relLabel;
    QLabel* u_iconLabel;
    QLabel* u_parentLabel;
    QComboBox *u_options;
};

#endif // REPLACEWIDGET_H
