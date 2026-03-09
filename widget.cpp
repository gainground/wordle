#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QDateTime>
#include <QKeyEvent>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , mAttempts(0)
    , mTotalScore(0)
    , mGameActive(false)
    , mWaitingForContinue(false)
{
    ui->setupUi(this);

    // три кнопки меню: меню, рекорды, игра
    ui->gameMenuButton->setCheckable(true);
    ui->recordsMenuButton->setCheckable(true);
    ui->gameMenuButton->setChecked(true);

    connect(ui->gameMenuButton, &QPushButton::clicked, this, &Widget::on_gameMenuButton_clicked);
    connect(ui->recordsMenuButton, &QPushButton::clicked, this, &Widget::on_recordsMenuButton_clicked);
    connect(ui->newGameButton, &QPushButton::clicked, this, &Widget::on_newGameButton_clicked);
    connect(ui->checkButton, &QPushButton::clicked, this, &Widget::on_checkButton_clicked);

    loadWordsFromFile();

    ui->wordInput->setEnabled(false);
    ui->checkButton->setEnabled(false);
    ui->logText->append("WORDLE");
    ui->logText->append("Нажмите 'Новая игра' для начала");

    loadRecords();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::loadWordsFromFile()
{
    QFile file("words.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        while (!in.atEnd()) {
            QString word = in.readLine().trimmed().toLower();
            if (word.length() == 4) {
                mWordsList.append(word);
            }
        }
        file.close();
    }

    if (mWordsList.isEmpty()) {
        mWordsList = QStringList()
            << QString::fromUtf8("рука")
            << QString::fromUtf8("лось")
            << QString::fromUtf8("барс")
            << QString::fromUtf8("нога")
            << QString::fromUtf8("ночь")
            << QString::fromUtf8("корь")
            << QString::fromUtf8("перо")
            << QString::fromUtf8("море")
            << QString::fromUtf8("гора")
            << QString::fromUtf8("река");
    }
}

void Widget::saveScore()
{
    if (mTotalScore == 0) return;

    QFile file("scores.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")
            << " - " << mTotalScore << " баллов\n";
        file.close();
    }
}

void Widget::loadRecords()
{
    ui->recordsList->clear();

    QVector<int> scores;

    QFile file("scores.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setCodec("UTF-8");

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (!line.isEmpty()) {
                QStringList parts = line.split(" - ");
                if (parts.size() == 2) {
                    QString scoreStr = parts[1].split(" ")[0];
                    int score = scoreStr.toInt();
                    scores.append(score);
                }
            }
        }
        file.close();
    }

    // сортировка для рекордов убывание
    for (int i = 0; i < scores.size() - 1; i++) {
        for (int j = 0; j < scores.size() - i - 1; j++) {
            if (scores[j] < scores[j + 1]) {
                int tempScore = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = tempScore;
            }
        }
    }

    // отсеиваем 3 рекорда
    if (scores.isEmpty()) {
        ui->recordsList->addItem("Пока нет рекордов");
    } else {
        int count = qMin(scores.size(), 3);  
        for (int i = 0; i < count; i++) {
            QString place;
            if (i == 0) place = "1 место:";
            else if (i == 1) place = "2 место:";
            else if (i == 2) place = "3 место:";

            QString record = QString("%1 %2 баллов")
                                 .arg(place)
                                 .arg(scores[i]);
            ui->recordsList->addItem(record);
        }
    }
}
