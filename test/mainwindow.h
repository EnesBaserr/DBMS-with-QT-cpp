#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QVBoxLayout>
#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QDate>
#include <QSqlDatabase>
#include <QVector>
#include <QScrollArea>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public:
    QStackedWidget* stackedWidget;
    QVBoxLayout* managerLayout;
    QVBoxLayout* coachLayout;
    QVBoxLayout *coachPageLayout;
    QWidget *coachPageContent;
    QVBoxLayout *managerPageLayout;
    QWidget *managerPageContent;
    QVBoxLayout* juryLayout;
    QVBoxLayout* playerLayout;
    QPushButton* logoutButton;
    QPushButton* logoutButton2;
    QPushButton* logoutButton3;
    QPushButton* logoutButton4;
    QLineEdit* usernameInput;
    QLineEdit* passwordInput;
    QSqlDatabase db;
    QScrollArea *scrollArea;
    QVector<QComboBox*> playerComboBoxes;
    QVector<QComboBox*> positionComboBoxes;
    void addPlayer(const QString& username, const QString& password, const QString& name, const QString& surname, const QString& userType, const QDate& dob, double height, double weight, const QString& nationality, int position, int selectedTeamId) ;
    void deleteMatchSession(const QString& sessionId);
    void clearLayout(QLayout *layout);
    void updateStadiumName(const QString& oldName, const QString& newName);
    void showStadiums();
    void setupCoachPage() ;
    void setupManagerPage();
    void addJury(const QString& username, const QString& password, const QString& name, const QString& surname, const QString& userType,  const QString& nationality);
    bool checkSomeCondition();  // Example condition checker
    void navigatePages();
    void setupJuryPage();
    void showRatingsAndCount();
    void addCoach(const QString& username, const QString& password, const QString& name, const QString& surname, const QString& userType, const QString& nationality) ;
    void submitRating(int sessionId, double rating);
    void setupPlayerPage();
    void loadTeams(QComboBox* comboBox);
    void loadStadiums(QComboBox* comboBox);
    void loadJuries(QComboBox* comboBox);
    void setupDateEdit(QDateEdit* dateEdit, const QString& currentCoachUsername);
    void addMatchSession(const QString& teamId, const QDate& date, const QString& timeSlot, const QString& stadium, const QString& juryName);
    void setupCoachSquadPage();
    void loadSessionsForCoach(QComboBox* comboBox);
    void loadPlayersForTeam(QComboBox* playerBox, QComboBox* positionBox, int position);
    void submitSquad(const QString& sessionId);
};

#endif // MAINWINDOW_H
