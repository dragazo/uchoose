#include "nodeeditor.h"
#include "ui_nodeeditor.h"

NodeEditor::NodeEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeEditor)
{
    ui->setupUi(this);
}

NodeEditor::~NodeEditor()
{
    delete ui;
}

// -- accessors -- //

QString NodeEditor::title() const
{
    return ui->TitleText->text();
}
void NodeEditor::title(const QString &str)
{
    ui->TitleText->setText(str);
}

QString NodeEditor::text() const
{
    return ui->TextText->toPlainText();
}
void NodeEditor::text(const QString &str)
{
    ui->TextText->setPlainText(str);
}

// -- button impl -- //

void NodeEditor::on_CancelButton_clicked()
{
    reject();
}

void NodeEditor::on_OKButton_clicked()
{
    accept();
}
