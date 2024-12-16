#include "metricreporter.h"


MetricReporter::MetricReporter(QWidget *parent) : QFrame(parent)
{
    /* так как все эти элементы добавляются как виджеты,
       они будут автоматически удалены деструктором QSplitter */
    wordLabel = new QLabel("Words: ");
    wordCountLabel = new QLabel();
    charLabel = new QLabel("Chars: ");
    charCountLabel = new QLabel();
    lineLabel = new QLabel("Line: ");
    lineCountLabel = new QLabel();
    columnLabel = new QLabel("Column: ");
    columnCountLabel = new QLabel();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(wordLabel);
    layout->addWidget(wordCountLabel);
    layout->addWidget(charLabel);
    layout->addWidget(charCountLabel);
    layout->addWidget(lineLabel);
    layout->addWidget(lineCountLabel);
    layout->addWidget(columnLabel);
    layout->addWidget(columnCountLabel);
    setLayout(layout);
}


// Обновляет количество слов с использованием соответствующей метки.
void MetricReporter::updateWordCount(int wordCount)
{
    wordCountLabel->setText(QString::number(wordCount));
}


// Обновляет количество символов с использованием соответствующей метки.
void MetricReporter::updateCharCount(int charCount)
{
    charCountLabel->setText(QString::number(charCount));
}


// Обновляет номер строки с использованием соответствующей метки.
void MetricReporter::updateLineCount(int current, int total)
{
    lineCountLabel->setText(QString::number(current) + tr("/") + QString::number(total));
}


// Обновляет номер колонки с использованием соответствующей метки.
void MetricReporter::updateColumnCount(int columnCount)
{
    columnCountLabel->setText(QString::number(columnCount));
}

