#include "StartWidget.h"
#include <QFont>
#include <QApplication>

StartWidget::StartWidget(QWidget *parent) : QWidget(parent)
{
    // 1. 设置开始界面大小和标题
    setWindowTitle("Qt TMX 游戏 - 开始界面");
    resize(800, 600); // 界面大小

    // 2. 创建标题（可选，美化用）
    m_titleLabel = new QLabel("嘉和苑摆摊大神", this);
    QFont titleFont("微软雅黑", 36, QFont::Bold); // 字体、大小、加粗
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter); // 居中显示

    // 3. 创建 Start, Exit 按钮
    m_startBtn = new QPushButton("Start Game", this);
    QFont btnFont("微软雅黑", 24, QFont::Normal);
    m_startBtn->setFont(btnFont);
    m_startBtn->setFixedSize(300, 80); // 固定按钮大小

    m_exitBtn = new QPushButton("Exit Game",this);
    m_exitBtn->setFont(btnFont);
    m_exitBtn->setFixedSize(300,80);

    // 4. 布局管理（垂直排列：标题在上，按钮在下）
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_startBtn, 0, Qt::AlignHCenter);
    layout->setSpacing(50); // 标题和按钮的间距
    layout->addWidget(m_exitBtn, 0, Qt::AlignHCenter);
    layout->setSpacing(50); // 退出和开始的间距
    layout->addStretch();
    layout->setContentsMargins(50, 50, 50, 50); // 界面边距

    // 5. 按钮点击信号连接：点击按钮发射 startGame 信号
    connect(m_startBtn, &QPushButton::clicked, this, &StartWidget::startGame);
    // 6. 按钮点击信号连接：点击按钮发射 exitGame 信号
    connect(m_exitBtn, &QPushButton::clicked, []() { // 退出应用
            QApplication::quit(); // 关闭整个程序
        });
}
