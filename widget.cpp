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
    if (file.open(QIODevice::Append | QIODevice::Text)) 
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << mTotalScore << "\n";
        file.close();
    }
}


void Widget::loadRecords()
{
    ui->recordsList->clear();
    QVector<int> scores;

    QFile file("scores.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        QTextStream in(&file);
        in.setCodec("UTF-8");

        while (!in.atEnd()) 
        {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) 
            {
                bool ok;
                int score = line.toInt(&ok);
                if (ok) 
                {
                    scores.append(score);
                }
            }
        }
        file.close();
    }

    // убывание
    std::sort(scores.begin(), scores.end(), std::greater<int>());

    // показываем топ-3
    if (scores.isEmpty()) 
    {
        ui->recordsList->addItem("Пока нет рекордов");
    } else {
        int count = qMin(scores.size(), 3);
        for (int i = 0; i < count; i++) 
        {
            QString place;
            if (i == 0) place = "1 место:";
            else if (i == 1) place = "2 место:";
            else if (i == 2) place = "3 место:";
            
            ui->recordsList->addItem(QString("%1 %2 баллов").arg(place).arg(scores[i]));
        }
    }
}

void Widget::startNewGame()
{
    if (mWordsList.isEmpty()) return;

    int randomIndex = QRandomGenerator::global()->bounded(mWordsList.size());
    mSecretWord = mWordsList[randomIndex];

    mAttempts = 0;
    mGameActive = true;
    mWaitingForContinue = false;

    // активация ввода
    ui->wordInput->setEnabled(true);
    ui->checkButton->setEnabled(true);
    ui->wordInput->clear();
    ui->wordInput->setFocus();

    // рестарт
    ui->logText->clear();
    ui->logText->append("game: Загаданное слово: ****");
}

void Widget::checkWord(const QString& word)
{
    if (word.length() != 4) {
        ui->logText->append("game: Слово должно быть длиной 4 буквы!");
        return;
    }

    mAttempts++;

    QString mask = "____";
    QStringList guessedLettersList;

    // проверка букв, если они на своих местах
    for (int i = 0; i < 4; i++) {
        if (word[i] == mSecretWord[i]) {
            mask[i] = word[i];
            QString letter = QString(word[i]);
            if (!guessedLettersList.contains(letter)) {
                guessedLettersList.append(letter);
            }
        }
    }

    // проверка букв, если они не на своих местах
    for (int i = 0; i < 4; i++) {
        if (word[i] != mSecretWord[i] && mSecretWord.contains(word[i])) {
            QString letter = QString(word[i]);
            if (!guessedLettersList.contains(letter)) {
                guessedLettersList.append(letter);
            }
        }
    }

    if (guessedLettersList.isEmpty()) {
        ui->logText->append("game: Отгаданные буквы: -");
    } else {
        ui->logText->append("game: Отгаданные буквы: " + guessedLettersList.join(", "));
    }
    ui->logText->append("game: Слово: " + mask);

    if (word == mSecretWord) {
        int points = 0;
        switch(mAttempts) {
            case 1: points = 5; break;
            case 2: points = 4; break;
            case 3: points = 3; break;
            case 4: points = 2; break;
            case 5: points = 1; break;
        }

        mTotalScore += points;
        ui->scoreLabel->setText(QString("Счет: %1").arg(mTotalScore));

        ui->logText->append(QString("game: Вы отгадали слово и заработали %1 баллов. Желаете продолжить? (Y or N)").arg(points));

        mWaitingForContinue = true;
        ui->wordInput->setEnabled(false);
        ui->checkButton->setEnabled(false);
    }
    else if (mAttempts >= 5) {
        ui->logText->append(QString("game: Попытки закончились, игра окончена. Ваш счет: %1 баллов").arg(mTotalScore));
        endGame();
    }
}

void Widget::endGame()
{
    saveScore();  

    mTotalScore = 0;
    ui->scoreLabel->setText("Счет: 0");

    mGameActive = false;
    ui->wordInput->setEnabled(false);
    ui->checkButton->setEnabled(false);
    loadRecords();
}

void Widget::on_gameMenuButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->gameMenuButton->setChecked(true);
    ui->recordsMenuButton->setChecked(false);
}

void Widget::on_recordsMenuButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->recordsMenuButton->setChecked(true);
    ui->gameMenuButton->setChecked(false);
    loadRecords();
}

void Widget::on_newGameButton_clicked()
{
    if (mGameActive) {
        // если игра активна, завершаем её и сохраняем счет
        endGame();
    }

    startNewGame();
}

void Widget::on_checkButton_clicked()
{
    if (!mGameActive || mWaitingForContinue) return;

    QString word = ui->wordInput->text().toLower();

    if (word.isEmpty()) {
        return;
    }

    ui->wordInput->clear();
    ui->logText->append("player: " + word);
    checkWord(word);
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if (mWaitingForContinue) {
        if (event->text() == "y" || event->text() == "Y" ||
            event->text() == "н" || event->text() == "Н") {
            mWaitingForContinue = false;
            startNewGame();  // продолңаем игру с имеюәимся счетом
        }
        else if (event->text() == "n" || event->text() == "N" ||
                 event->text() == "т" || event->text() == "Т") {
            mWaitingForContinue = false;
            ui->logText->append(QString("game: Вы заработали %1 баллов.").arg(mTotalScore));
            endGame();  // обнуляемся
        }
    }
    QWidget::keyPressEvent(event);
}