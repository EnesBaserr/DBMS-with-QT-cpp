#include "mainwindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QLabel>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    // Create the stacked widget
    this->setFixedSize(1200, 900);
    qDebug()<< QSqlDatabase::drivers();
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setPort(3306);  // Örnek olarak varsayılan MySQL port numarası

    db.setHostName("localhost");
    db.setDatabaseName("project3_test");
    db.setUserName("root");
    db.setPassword("admin");

    db.open();
    if (!db.open()) {
        QMessageBox::critical(this, "Database Connection", "Unable to connect to database.");
        return;
    }

    stackedWidget = new QStackedWidget(this);

    // Initial page with username input
    QWidget* loginPage = new QWidget();
    QVBoxLayout* loginLayout = new QVBoxLayout(loginPage);
    usernameInput = new QLineEdit();
    passwordInput = new QLineEdit();
    QPushButton* loginButton = new QPushButton("Login");
    loginLayout->addWidget(new QLabel("Enter Username:"));
    loginLayout->addWidget(usernameInput);
    loginLayout->addWidget(new QLabel("Enter Password:"));
    loginLayout->addWidget(passwordInput);
    loginLayout->addWidget(loginButton);
    stackedWidget->addWidget(loginPage);
    // Welcome pages
    QWidget* welcomeManager = new QWidget();
    managerLayout = new QVBoxLayout(welcomeManager);

    stackedWidget->addWidget(welcomeManager);

    QWidget* welcomeCoach = new QWidget();
    coachLayout = new QVBoxLayout(welcomeCoach);

    stackedWidget->addWidget(welcomeCoach);
    // -------------- Jury
    QWidget* welcomeJury = new QWidget();
    juryLayout = new QVBoxLayout(welcomeJury); 
    stackedWidget->addWidget(welcomeJury);
   // -------------- Player
    QWidget* welcomePlayer = new QWidget();
    playerLayout = new QVBoxLayout(welcomePlayer);
    stackedWidget->addWidget(welcomePlayer);

    // Connect login button to navigate pages method
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::navigatePages);
    // Layout to hold the stacked widget
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(stackedWidget);
}
void MainWindow::clearLayout(QLayout *layout) {
    if (!layout) return;  // Early exit if the layout is null

    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            delete widget;
        } else if (QLayout *childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }
        delete item;
    }
}


// Coach Related
void MainWindow::setupCoachPage() {
    QLabel* deleteInstructionsLabel = new QLabel("Enter Session ID to delete match session:");
    coachLayout->addWidget(deleteInstructionsLabel);

    QLineEdit* sessionIdInput = new QLineEdit();
    sessionIdInput->setPlaceholderText("Enter Session ID");
    coachLayout->addWidget(sessionIdInput);

    QPushButton *deleteSessionButton = new QPushButton("Delete Match Session");
    coachLayout->addWidget(deleteSessionButton);
    connect(deleteSessionButton, &QPushButton::clicked, [this, sessionIdInput]() {
        QString sessionId = sessionIdInput->text();
        if (sessionId.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Session ID cannot be empty.");
            return;
        }
        deleteMatchSession(sessionId);
    });
    QPushButton *showStadiumsButton = new QPushButton("Show Stadiums");
    coachLayout->addWidget(showStadiumsButton);

    connect(showStadiumsButton, &QPushButton::clicked, this, &MainWindow::showStadiums);
}
void MainWindow::deleteMatchSession(const QString& sessionId) {
    QString username = usernameInput->text();  // Assuming the coach's username is stored here after login

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT t.coach_username FROM teams t "
                       "JOIN matchsessions ms ON ms.team_id = t.team_id "
                       "WHERE ms.session_id = :sessionId");
    bool ok;
    int session_id_int = sessionId.toInt(&ok);
    checkQuery.bindValue(":sessionId", session_id_int);

    if (checkQuery.exec() && checkQuery.next()) {
        QString coachUsername = checkQuery.value(0).toString();
        if (username != coachUsername) {
            QMessageBox::warning(this, "Permission Denied", "You can only delete sessions of your own teams.");
            return;
        }

        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM matchsessions WHERE session_id = :sessionId");
        deleteQuery.bindValue(":sessionId", session_id_int);
        if (deleteQuery.exec()) {
            QMessageBox::information(this, "Deletion Successful", "Match session and all related data deleted successfully.");
        } else {
            QMessageBox::critical(this, "Deletion Failed", "Failed to delete the match session.");
        }
    } else {
        QMessageBox::warning(this, "Not Found", "No match session found with the provided ID or you do not have the right to delete it.");
    }
}

void MainWindow::showStadiums() {
    QDialog *dialog = new QDialog(this);

    dialog->setWindowTitle("Stadiums List");
    QTableWidget *table = new QTableWidget(dialog);
    dialog->resize(600, 400);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList() << "Stadium Name" << "Country");
    table->horizontalHeader()->setStretchLastSection(false); // Stretch the last column to fill the space
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSqlQuery query;
    if (query.exec("SELECT stadium_name, stadium_country FROM stadiums")) {
        int row = 0;
        while (query.next()) {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            row++;
        }
    }
    table->setColumnWidth(0, 400);  // Set the width of the "Stadium Name" column to 400
    table->setColumnWidth(1, 180);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(table);
    dialog->setLayout(layout);
    dialog->exec();
}
// Jury Related
void MainWindow::setupJuryPage() {
    QPushButton *showRatingButton = new QPushButton("Show Rating avg. & count");
    juryLayout->addWidget(showRatingButton);
    QLabel* instructionsLabel = new QLabel("Select a match session and rate it:");
    juryLayout->addWidget(instructionsLabel);

    // Combo box for session IDs
    QComboBox* sessionComboBox = new QComboBox();
    QLineEdit* ratingInput = new QLineEdit();
    QPushButton* rateButton = new QPushButton("Submit Rating");

    // Populate the combo box with eligible session IDs
    QString username = usernameInput->text();
    QSqlQuery query;
    query.prepare("SELECT session_id, date FROM matchsessions WHERE assigned_jury_username = :username AND rating IS NULL AND date < CURRENT_DATE()");
    query.bindValue(":username", username);
    if (query.exec()) {
        while (query.next()) {
            QString sessionInfo = QString("Session ID: %1, Date: %2").arg(query.value(0).toString(), query.value(1).toString());
            sessionComboBox->addItem(sessionInfo, query.value(0));  // Store session_id as userData for easy access
        }
    }

    juryLayout->addWidget(sessionComboBox);
    ratingInput->setPlaceholderText("Enter your rating (0.0 - 5.0)");
    juryLayout->addWidget(ratingInput);
    juryLayout->addWidget(rateButton);

    connect(rateButton, &QPushButton::clicked, [this, sessionComboBox, ratingInput]() {
        QVariant sessionId = sessionComboBox->currentData();
        bool ok;
        double rating = ratingInput->text().toDouble(&ok);
        if (!ok || rating < 0.0 || rating > 5.0) {
            QMessageBox::warning(this, "Input Error", "Please enter a valid rating between 0.0 and 5.0.");
            return;
        }
        submitRating(sessionId.toInt(), rating);
    });

    connect(showRatingButton, &QPushButton::clicked, this, &MainWindow::showRatingsAndCount);
}
void MainWindow::showRatingsAndCount(){
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Rating Info");
    QTableWidget *table = new QTableWidget(dialog);
    dialog->resize(600, 400);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList() << "Average Rating" << "Total Count");
    table->horizontalHeader()->setStretchLastSection(false); // Stretch the last column to fill the space
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSqlQuery query;
    QString username = usernameInput->text();
    query.prepare("SELECT AVG(rating), COUNT(*) FROM matchsessions WHERE assigned_jury_username = :username and rating is not null");
    query.bindValue(":username", username);
    if (query.exec()) {
        int row = 0;
        while (query.next()) {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            row++;
        }
    }
    table->setColumnWidth(0, 400);  // Set the width of the "Stadium Name" column to 400
    table->setColumnWidth(1, 180);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(table);
    dialog->setLayout(layout);
    dialog->exec();

}
void MainWindow::submitRating(int sessionId, double rating) {
    QSqlQuery query;
    query.prepare("UPDATE matchsessions SET rating = :rating WHERE session_id = :sessionId AND rating IS NULL AND date < CURRENT_DATE()");
    query.bindValue(":rating", rating);
    query.bindValue(":sessionId", sessionId);
    if (query.exec()) {
        if (query.numRowsAffected() > 0) {
            QMessageBox::information(this, "Rating Submitted", "Your rating has been successfully submitted.");
        } else {
            QMessageBox::warning(this, "Rating Not Submitted", "This session has already been rated or is not eligible for rating.");
        }
    } else {
        QMessageBox::critical(this, "Submission Failed", QString("Failed to submit your rating"));
    }
}
void MainWindow::setupManagerPage() {
    QLabel* instructionsLabel = new QLabel("Select a stadium and update its name:");
    managerLayout->addWidget(instructionsLabel);

    // ComboBox to display stadium names
    QComboBox* stadiumComboBox = new QComboBox();
    QSqlQuery query;
    if (query.exec("SELECT stadium_name FROM stadiums")) {
        while (query.next()) {
            stadiumComboBox->addItem(query.value(0).toString());
        }
    }
    managerLayout->addWidget(stadiumComboBox);

    // Input field for new stadium name
    QLineEdit* newNameInput = new QLineEdit();
    newNameInput->setPlaceholderText("Enter new stadium name");
    managerLayout->addWidget(newNameInput);

    // Button to update stadium name
    QPushButton* updateButton = new QPushButton("Update Stadium Name");
    managerLayout->addWidget(updateButton);
    connect(updateButton, &QPushButton::clicked, [this, stadiumComboBox, newNameInput]() {
        QString oldName = stadiumComboBox->currentText();
        QString newName = newNameInput->text();
        if (newName.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "New stadium name cannot be empty.");
            return;
        }
        updateStadiumName(oldName, newName);
    });
}

void MainWindow::updateStadiumName(const QString& oldName, const QString& newName) {
    QSqlQuery query;
    query.prepare("UPDATE stadiums SET stadium_name = :newName WHERE stadium_name = :oldName");
    query.bindValue(":newName", newName);
    query.bindValue(":oldName", oldName);
    if (query.exec()) {
        QMessageBox::information(this, "Update Successful", "Stadium name updated successfully.");
    } else {
        QMessageBox::critical(this, "Update Failed", "Failed to update the stadium name.");
    }
}
void MainWindow::navigatePages() {
    QString username = usernameInput->text();
    QString password = passwordInput->text();
    QSqlQuery query;
    query.prepare("SELECT user_type FROM users WHERE binary username = :username AND binary password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    QSqlQuery query2;
    query2.prepare("SELECT username FROM database_managers WHERE binary username = :username AND binary password = :password");
    query2.bindValue(":username", username);
    query2.bindValue(":password", password);
    if(query2.exec() && query2.next()){
        QLabel* welcomeLabel = new QLabel(QString("Welcome, Manager %1!").arg(username));
        managerLayout->addWidget(welcomeLabel);
        setupManagerPage();
        logoutButton = new QPushButton("Log out");
        managerLayout->addWidget(logoutButton);
        connect(logoutButton, &QPushButton::clicked, [this]() {
            clearLayout(managerLayout);
            stackedWidget->setCurrentIndex(0);  // Navigate back to login page
        });
        stackedWidget->setCurrentIndex(1);  // Navigate to Welcome Manager page
    }

    else if(query.exec() && query.next()){
        QString role = query.value(0).toString();
        if (role == "manager") {



        } else if (role == "coach") {
            QLabel* welcomeLabel = new QLabel(QString("Welcome, Coach %1!").arg(username));
            coachLayout->addWidget(welcomeLabel);
            setupCoachPage();
            logoutButton2 = new QPushButton("Log out");

            coachLayout->addWidget(logoutButton2);
            connect(logoutButton2, &QPushButton::clicked, [this]() {
                clearLayout(coachLayout);
                stackedWidget->setCurrentIndex(0);  // Navigate back to login page
            });
            stackedWidget->setCurrentIndex(2);  // Navigate to Welcome Coach page


        }
        else if(role == "jury") {
            QLabel* welcomeLabel = new QLabel(QString("Welcome, Jury %1!").arg(username));
            juryLayout->addWidget(welcomeLabel);
            setupJuryPage();
            logoutButton3 = new QPushButton("Log out");
            juryLayout->addWidget(logoutButton3);
            connect(logoutButton3, &QPushButton::clicked, [this]() {
                clearLayout(juryLayout);
                stackedWidget->setCurrentIndex(0);  // Navigate back to login page
            });
            stackedWidget->setCurrentIndex(3);  // Navigate to Welcome Coach pag

        }
        else if(role == "player") {
            QLabel* welcomeLabel = new QLabel(QString("Welcome, Player %1!").arg(username));
            playerLayout->addWidget(welcomeLabel);
            logoutButton4 = new QPushButton("Log out");
            playerLayout->addWidget(logoutButton4);
            connect(logoutButton4, &QPushButton::clicked, [this]() {
                clearLayout(playerLayout);
                stackedWidget->setCurrentIndex(0);  // Navigate back to login page
            });
            stackedWidget->setCurrentIndex(4);  // Navigate to Welcome Coach pag

        }

    }
    else{
        QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
    }
}
