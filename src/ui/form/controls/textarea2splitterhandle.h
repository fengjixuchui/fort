#ifndef TEXTAREA2SPLITTERHANDLE_H
#define TEXTAREA2SPLITTERHANDLE_H

#include <QSplitterHandle>

QT_FORWARD_DECLARE_CLASS(QBoxLayout)
QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)

QT_FORWARD_DECLARE_CLASS(TextArea2Splitter)

class TextArea2SplitterHandle : public QSplitterHandle
{
    Q_OBJECT

public:
    explicit TextArea2SplitterHandle(Qt::Orientation o, QSplitter *parent);

    QPlainTextEdit *textArea1() const { return m_textArea1; }
    void setTextArea1(QPlainTextEdit *v) { m_textArea1 = v; }

    QPlainTextEdit *textArea2() const { return m_textArea2; }
    void setTextArea2(QPlainTextEdit *v) { m_textArea2 = v; }

    QPlainTextEdit *currentTextArea() const;

    QPushButton *btMoveAllFrom1To2() const { return m_btMoveAllFrom1To2; }
    QPushButton *btMoveSelectedFrom1To2() const { return m_btMoveSelectedFrom1To2; }
    QPushButton *btInterchangeAll() const { return m_btInterchangeAll; }
    QPushButton *btMoveSelectedFrom2To1() const { return m_btMoveSelectedFrom2To1; }
    QPushButton *btMoveAllFrom2To1() const { return m_btMoveAllFrom2To1; }

    TextArea2Splitter *splitter() const;

    QBoxLayout *buttonsLayout() const { return m_buttonsLayout; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();

private:
    QPlainTextEdit *m_textArea1 = nullptr;
    QPlainTextEdit *m_textArea2 = nullptr;

    QBoxLayout *m_buttonsLayout = nullptr;
    QPushButton *m_btMoveAllFrom1To2 = nullptr;
    QPushButton *m_btMoveSelectedFrom1To2 = nullptr;
    QPushButton *m_btInterchangeAll = nullptr;
    QPushButton *m_btMoveSelectedFrom2To1 = nullptr;
    QPushButton *m_btMoveAllFrom2To1 = nullptr;
};

#endif // TEXTAREA2SPLITTERHANDLE_H
