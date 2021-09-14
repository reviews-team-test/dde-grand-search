/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "textview.h"

#include <QHBoxLayout>
#include <QScrollBar>
#include <QTextCodec>
#include <QTimer>
#include <QPainter>

void PlainTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    //解决鼠标按住向下滑动，当超出本控件区域后会触发scrollbar滚动
    //屏蔽该事件，不再激活autoScrollTimer
    if (e->source() == Qt::MouseEventNotSynthesized) {
        e->accept();
        return;
    }

    QPlainTextEdit::mouseMoveEvent(e);
}

QString TextView::toUnicode(const QByteArray &data)
{
    QString text;
    if (data.isEmpty())
        return text;

    //优先判断
    static const QByteArrayList maybe = {"UTF-8", "GBK"};
    for (const QByteArray &code : maybe) {
        if (QTextCodec *codec = QTextCodec::codecForName(code)) {
            QTextCodec::ConverterState state;
            text = codec->toUnicode(data.constData(), data.size(), &state);
            if (state.invalidChars < 1)
                return text;
        }
    }

    return QString::fromLocal8Bit(data);
}

void TextView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    //文本框的背景
    auto view = m_browser->viewport();
    painter.setBrush(view->palette().color(view->backgroundRole()));
    painter.setPen(Qt::NoPen);

    //画圆角背景,背景大小为去除左边距10的区域
    auto r = rect();
    r.setLeft(10);
    painter.drawRoundedRect(r, 8, 8);

    QWidget::paintEvent(event);
}


TextView::TextView(QWidget *parent) : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    this->setLayout(layout);

    layout->setSpacing(0);
    this->setMinimumHeight(386);

    m_browser = new PlainTextEdit(this);

    //文本界面不绘制背景，自绘圆角背景
    m_browser->viewport()->setAutoFillBackground(false);
    m_browser->setFrameShape(QFrame::NoFrame);

    //无滚动
    m_browser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_browser->verticalScrollBar()->setDisabled(true);
    m_browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_browser->horizontalScrollBar()->setDisabled(true);

    //无交互，自动换行
    m_browser->setReadOnly(true);
    m_browser->setTextInteractionFlags(Qt::NoTextInteraction);
    m_browser->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_browser->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_browser->setFocusPolicy(Qt::NoFocus);

    //样式
    //文本内容上边距是10,左右边距是20,通过DocumentMargin设置10的边距
    //再通过layout增加左右边距各10，来达到上边距与左右边距不一样的效果
    m_browser->document()->setDocumentMargin(10);
    //左边距文本背景与中线10,加上文本内容边距10;补充文本边距10
    //因此在绘制圆角背景时为只去除左边距离中线10的区域
    layout->setContentsMargins(10 + 10, 0, 0 + 10, 0);

    layout->addWidget(m_browser);
}

void TextView::setSource(const QString &path)
{
    m_browser->clear();

    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        auto datas = file.read(2048);
        m_browser->setPlainText(toUnicode(datas));
    }
}
