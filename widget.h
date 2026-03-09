#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStringList>
#include <QKeyEvent>
#include <QFileInfo>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_gameMenuButton_clicked();
    void on_recordsMenuButton_clicked();
    void on_newGameButton_clicked();
    void on_checkButton_clicked();

private:
    Ui::Widget *ui;

    QString mSecretWord;
    QStringList mWordsList;
    int mAttempts;
    int mTotalScore;
    bool mGameActive;
    bool mWaitingForContinue;

    void loadWordsFromFile();
    void startNewGame();
    void checkWord(const QString& word);
    void endGame();
    void saveScore();
    void loadRecords();
};

#endif // WIDGET_H
