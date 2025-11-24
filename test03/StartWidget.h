#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class StartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StartWidget(QWidget *parent = nullptr);

signals:
    // 点击 Start 按钮后发射的信号，用于通知主函数切换界面
    void startGame();

private:
    QPushButton *m_startBtn; // Start 按钮
    QPushButton *m_exitBtn;
    QLabel *m_titleLabel;    // 游戏标题
};

#endif // STARTWIDGET_H
