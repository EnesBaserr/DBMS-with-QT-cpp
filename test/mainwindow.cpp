#include "mainwindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QLabel>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QComboBox>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPixmap>
#include <QScrollArea>


MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    // Create the stacked widget
    this->setFixedSize(900, 900);

    qDebug()<< QSqlDatabase::drivers();
     db = QSqlDatabase::addDatabase("QMYSQL");
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
    usernameInput->setPlaceholderText(QString("Please enter username"));
    usernameInput->setFixedHeight(40);
    QFont usernameFont = usernameInput->font();
    usernameFont.setPointSize(12);
    usernameFont.setBold(true);
    usernameInput->setFont(usernameFont);
    passwordInput = new QLineEdit();
    passwordInput->setFixedHeight(40);
    passwordInput->setPlaceholderText(QString("Please enter password"));
    QFont passwordFont = passwordInput->font();
    passwordFont.setPointSize(12);
    passwordFont.setBold(true);
    passwordInput->setFont(passwordFont);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* imageLabel = new QLabel();
    QPixmap pixmap(":/data-server.png"); // Replace with the actual path or resource link
    imageLabel->setPixmap(pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation)); // Adjust the size as needed

    QLabel* headerLabel = new QLabel("Welcome to DBMS");
    headerLabel->setAlignment(Qt::AlignCenter);
    QFont font = headerLabel->font();
    font.setPointSize(24);
    font.setBold(true);
    headerLabel->setFont(font);
    headerLabel->setStyleSheet("QLabel { color : #2c3e50; }");

    headerLayout->addWidget(imageLabel);
    headerLayout->addWidget(headerLabel);
    headerLayout->setAlignment(Qt::AlignCenter); // Center the header and image in the layout

    loginLayout->addLayout(headerLayout);
    QPushButton* loginButton = new QPushButton("Login");
     loginButton->setFixedSize(200, 50);

       loginButton->setCursor(Qt::PointingHandCursor);
     loginButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; border-radius: 5px; }"
                                "QPushButton:hover { background-color: #2980b9;  }");
       QLabel* loginLabel = new QLabel("Enter Username:");
     QFont font2 = loginLabel->font();
     font2.setPointSize(16);
     font2.setBold(true);
     loginLabel->setFont(font2);
     loginLabel->setStyleSheet("QLabel { color : #2c3e50; }");

    loginLayout->addWidget(loginLabel);
    loginLayout->addWidget(usernameInput);
    QLabel* passwordLabel = new QLabel("Enter Password:");
    QFont font3 = passwordLabel->font();
    font3.setPointSize(16);
    font3.setBold(true);
    passwordLabel->setFont(font2);
    passwordLabel->setStyleSheet("QLabel { color : #2c3e50; }");
    loginLayout->addWidget(passwordLabel);
     passwordInput->setEchoMode(QLineEdit::Password);
    loginLayout->addWidget(passwordInput);
    loginLayout->addWidget(loginButton, 0, Qt::AlignCenter);

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
void MainWindow::setupPlayerPage() {
    QLabel* label = new QLabel("Players you have played with:");
    playerLayout->addWidget(label);

    QTableWidget* table = new QTableWidget();
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(QStringList() << "Username" << "Surname" << "Height" << "Count");

    QString currentPlayer = usernameInput->text();  // Assumes the player's username is stored here

    // First, find the maximum session count
    int maxSessionCount = 0;
    QSqlQuery maxCountQuery;
    maxCountQuery.prepare("SELECT MAX(count) FROM (SELECT COUNT(*) AS count FROM sessionsquad ss1 JOIN sessionsquad ss2 ON ss1.session_id = ss2.session_id AND ss1.played_player_username != ss2.played_player_username WHERE ss1.played_player_username = :current_player GROUP BY ss2.played_player_username) AS session_counts");
    maxCountQuery.bindValue(":current_player", currentPlayer);
    if (maxCountQuery.exec() && maxCountQuery.next()) {
        maxSessionCount = maxCountQuery.value(0).toInt();
    }

    // Populate the table
    QSqlQuery query;
    query.prepare("SELECT ss2.played_player_username, p2.surname, p2.height, COUNT(*) FROM sessionsquad ss1 JOIN sessionsquad ss2 ON ss1.session_id = ss2.session_id AND ss1.played_player_username != ss2.played_player_username JOIN users p1 ON ss1.played_player_username = p1.username JOIN users p2 ON ss2.played_player_username = p2.username WHERE ss1.played_player_username = :current_player GROUP BY ss2.played_player_username, p2.surname, p2.height ORDER BY COUNT(*) DESC");
    query.bindValue(":current_player", currentPlayer);

    if (query.exec()) {
        int row = 0;
        while (query.next()) {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
            table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));

            // Highlight the row if the session count matches the max session count
            if (query.value(3).toInt() == maxSessionCount) {
                for (int column = 0; column < 4; ++column) {
                    table->item(row, column)->setBackground(Qt::yellow);
                }
            }
            ++row;
        }
    } else {
        QMessageBox::critical(this, "Error", "Failed to retrieve data: " );
    }

    playerLayout->addWidget(table);

    // Calculate and display the average height of the most frequently played with player(s)
    QLabel* heightLabel = new QLabel("Average height of most frequently played with player(s):");
    playerLayout->addWidget(heightLabel);
    QLineEdit* heightDisplay = new QLineEdit();
    heightDisplay->setReadOnly(true);
    QSqlQuery heightQuery;
    heightQuery.prepare(R"(
        SELECT AVG(subquery.height) AS average_height
        FROM (
            SELECT
                ss2.played_player_username,
                p.height,
                COUNT(*) AS num_sessions
            FROM
                sessionsquad ss1
            JOIN
                sessionsquad ss2 ON ss1.session_id = ss2.session_id AND ss1.played_player_username != ss2.played_player_username
            JOIN
                users p ON ss2.played_player_username = p.username
            WHERE
                ss1.played_player_username = :current_player
            GROUP BY
                ss2.played_player_username, p.height
        ) AS subquery
        WHERE
            subquery.num_sessions = (
                SELECT MAX(num_sessions)
                FROM (
                    SELECT
                        COUNT(*) AS num_sessions
                    FROM
                        sessionsquad ss1
                    JOIN
                        sessionsquad ss2 ON ss1.session_id = ss2.session_id AND ss1.played_player_username != ss2.played_player_username
                    WHERE
                        ss1.played_player_username = :current_player
                    GROUP BY
                        ss2.played_player_username
                ) AS count_query
            )
    )");
    heightQuery.bindValue(":current_player", currentPlayer);

    if (heightQuery.exec() && heightQuery.next()) {
        QString averageHeight = heightQuery.value(0).toString();
        heightDisplay->setText(averageHeight + " cm");
    } else {
        heightDisplay->setText("Data not available");
    }

    playerLayout->addWidget(heightDisplay);
}





// Coach Related
void MainWindow::loadTeams(QComboBox* comboBox) {
    QSqlQuery query;
    QString currentCoachUsername = usernameInput->text();  // Assuming the coach's username is stored here
    query.prepare("SELECT team_id FROM teams WHERE coach_username = :coach_username AND contract_start<= CURRENT_DATE() AND contract_finish >= CURRENT_DATE()");
    query.bindValue(":coach_username", currentCoachUsername);
    if (query.exec()) {
        while (query.next()) {
            comboBox->addItem(query.value(0).toString());
        }
    }
}

void MainWindow::loadStadiums(QComboBox* comboBox) {
    QSqlQuery query("SELECT stadium_name FROM stadiums");
    while (query.next()) {
        comboBox->addItem(query.value(0).toString());
    }
}
void MainWindow::setupDateEdit(QDateEdit* dateEdit, const QString& currentCoachUsername) {
    QSqlQuery query;
    query.prepare("SELECT contract_start, contract_finish FROM teams WHERE coach_username = :coach_username");
    query.bindValue(":coach_username", currentCoachUsername);
    if (query.exec()) {
        if (query.next()) {
            QDate contractStartDate = query.value(0).toDate();
            QDate contractEndDate = query.value(1).toDate();
            dateEdit->setMinimumDate(contractStartDate > QDate::currentDate() ? contractStartDate : QDate::currentDate());
            dateEdit->setMaximumDate(contractEndDate);
        } else {
            QMessageBox::warning(this, "Contract Error", "No valid contract dates found for the current coach.");
        }
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to retrieve contract dates: " );
    }
}

void MainWindow::loadJuries(QComboBox* comboBox) {
    QSqlQuery query("SELECT CONCAT(name, ' ', surname) AS full_name FROM users WHERE user_type = 'jury'");
    while (query.next()) {
        comboBox->addItem(query.value(0).toString());
    }
}
void MainWindow::addMatchSession(const QString& teamId, const QDate& date, const QString& timeSlot, const QString& stadium, const QString& juryName) {
    // Step 1: Fetch the maximum session_id
    int maxSessionId = 0;
    QSqlQuery maxIdQuery;
    if (maxIdQuery.exec("SELECT MAX(session_id) FROM matchsessions")) {
        if (maxIdQuery.next() && !maxIdQuery.value(0).isNull()) {
            maxSessionId = maxIdQuery.value(0).toInt();
        }
    } else {
        QMessageBox::critical(this, "Error", "Failed to retrieve max session ID: ");
        return;
    }

    // Step 2: Prepare the insertion query
    QSqlQuery query2;
    query2.prepare("SELECT stadium_id FROM stadiums where stadium_name = :stadium");
    query2.bindValue(":stadium", stadium);
    int stadium_id=0;


    if (query2.exec()&& query2.next()) {
        stadium_id=query2.value(0).toInt();
    }
    QSqlQuery query;
    query.prepare("INSERT INTO matchsessions (session_id, team_id, date, time_slot, stadium_id, assigned_jury_username) VALUES (:session_id, :team_id, :session_date, :time_slot, :stadium, (SELECT username FROM users WHERE CONCAT(name, ' ', surname) = :jury_name))");
    query.bindValue(":session_id", maxSessionId + 1);
    query.bindValue(":team_id", teamId);
    query.bindValue(":session_date", date);
    query.bindValue(":time_slot", timeSlot);
    query.bindValue(":stadium", stadium_id);
    query.bindValue(":jury_name", juryName);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to add session: " );
    } else {
        QMessageBox::information(this, "Success", "Match session added successfully.");
    }
}

void MainWindow::setupCoachPage() {
    coachPageContent = new QWidget();
    coachPageLayout = new QVBoxLayout(coachPageContent);

    QLabel* addSessionLabel = new QLabel("Add New Match Session:");
    coachPageLayout->addWidget(addSessionLabel);

    // ComboBox for team selection (filtered by current coach and contract validity)
    QLabel* teamLabel = new QLabel("Select Your Team:");
    QComboBox* teamComboBox = new QComboBox();
    coachPageLayout->addWidget(teamLabel);
    coachPageLayout->addWidget(teamComboBox);

    // Date selection within contract bounds
    QLabel* dateLabel = new QLabel("Select Date (within contract duration):");
    QDateEdit* dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    QString currentCoachUsername = usernameInput->text();  // Assumes the coach's username is stored here
    setupDateEdit(dateEdit, currentCoachUsername);  // Set up the date edit with contract dates
    coachPageLayout->addWidget(dateLabel);
    coachPageLayout->addWidget(dateEdit);

    // Time slot selection
    QLabel* timeSlotLabel = new QLabel("Select Time Slot:");
    QComboBox* timeSlotComboBox = new QComboBox();
    timeSlotComboBox->addItems({"1", "2", "3", "4"});  // Time slots as defined
    coachPageLayout->addWidget(timeSlotLabel);
    coachPageLayout->addWidget(timeSlotComboBox);

    // Stadium selection
    QLabel* stadiumLabel = new QLabel("Select Stadium:");
    QComboBox* stadiumComboBox = new QComboBox();
    coachPageLayout->addWidget(stadiumLabel);
    coachPageLayout->addWidget(stadiumComboBox);

    // Jury selection by name and surname
    QLabel* juryLabel = new QLabel("Assign Jury:");
    QComboBox* juryComboBox = new QComboBox();
    coachPageLayout->addWidget(juryLabel);
    coachPageLayout->addWidget(juryComboBox);

    QPushButton* addButton = new QPushButton("Add Match Session");
    coachPageLayout->addWidget(addButton);


    connect(addButton, &QPushButton::clicked, [this, teamComboBox, dateEdit, timeSlotComboBox, stadiumComboBox, juryComboBox]() {
        addMatchSession(teamComboBox->currentText(), dateEdit->date(), timeSlotComboBox->currentText(), stadiumComboBox->currentText(), juryComboBox->currentText());
    });

    loadTeams(teamComboBox);
    loadStadiums(stadiumComboBox);
    loadJuries(juryComboBox);

    // -------------------------------------------

    QLabel* sessionLabel = new QLabel("Select a Session:");
    QComboBox* sessionComboBox = new QComboBox();
    loadSessionsForCoach(sessionComboBox); // This will load session IDs managed by the coach

    coachPageLayout->addWidget(sessionLabel);
    coachPageLayout->addWidget(sessionComboBox);

    playerComboBoxes.resize(6);
    positionComboBoxes.resize(6);
    for (int i = 0; i < 6; ++i) {
        playerComboBoxes[i] = new QComboBox();
        positionComboBoxes[i] = new QComboBox();
        loadPlayersForTeam(playerComboBoxes[i], positionComboBoxes[i], i); // Loads players for the team and appropriate positions

        coachPageLayout->addWidget(new QLabel("Select Player " + QString::number(i + 1) + ":"));
        coachPageLayout->addWidget(playerComboBoxes[i]);
        coachPageLayout->addWidget(new QLabel("Select Position for Player " + QString::number(i + 1) + ":"));
        coachPageLayout->addWidget(positionComboBoxes[i]);
    }

    QPushButton* submitButton = new QPushButton("Submit Squad");
    coachPageLayout->addWidget(submitButton);
    connect(submitButton, &QPushButton::clicked, [this, sessionComboBox]() {


        submitSquad(sessionComboBox->currentText());
    });

    // -----------------------------------------
    QLabel* deleteInstructionsLabel = new QLabel("Enter Session ID to delete match session:");
    coachPageLayout->addWidget(deleteInstructionsLabel);

    QLineEdit* sessionIdInput = new QLineEdit();
    sessionIdInput->setPlaceholderText("Enter Session ID");
    coachPageLayout->addWidget(sessionIdInput);

    QPushButton *deleteSessionButton = new QPushButton("Delete Match Session");
    coachPageLayout->addWidget(deleteSessionButton);
    connect(deleteSessionButton, &QPushButton::clicked, [this, sessionIdInput]() {
        QString sessionId = sessionIdInput->text();
        if (sessionId.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Session ID cannot be empty.");
            return;
        }
        deleteMatchSession(sessionId);
    });
    QPushButton *showStadiumsButton = new QPushButton("Show Stadiums");
    coachPageLayout->addWidget(showStadiumsButton);
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);  // Make the scroll area content resizable
    scrollArea->setWidget(coachPageContent);  // Set the widget that contains all other widgets as the scroll area's widget

    // Now add the scroll area to the main layout of the coach page instead of directly adding widgets
    coachLayout->addWidget(scrollArea);  // coachLayout should be the layout that is part of the stackedWidget for the coach page

    connect(showStadiumsButton, &QPushButton::clicked, this, &MainWindow::showStadiums);
}

void MainWindow::loadSessionsForCoach(QComboBox* comboBox) {
    QString currentCoachUsername = usernameInput->text();  // Assuming coach's username is stored
    QSqlQuery query;
    query.prepare("SELECT session_id FROM matchsessions WHERE team_id IN (SELECT team_id FROM teams WHERE coach_username = :coach_username) AND session_id NOT IN (SELECT DISTINCT session_id FROM sessionsquad)");
    query.bindValue(":coach_username", currentCoachUsername);
    if (query.exec()) {
        while (query.next()) {
            comboBox->addItem(query.value(0).toString());
        }
    }
}

void MainWindow::loadPlayersForTeam(QComboBox* playerBox, QComboBox* positionBox, int position) {
    QString currentCoachUsername = usernameInput->text();
    QSqlQuery playerQuery;
    playerQuery.prepare(R"(
SELECT u.username, pp.position_id ,t.coach_username
    FROM users u
    INNER JOIN playerpositions pp ON u.username = pp.username
    INNER JOIN playerteams pt ON u.username = pt.username
    INNER JOIN teams t ON pt.team_id = t.team_id
WHERE t.coach_username = :coach_username
    AND t.contract_start <= CURRENT_DATE()
    AND t.contract_finish >= CURRENT_DATE()
)");
    playerQuery.bindValue(":coach_username", currentCoachUsername);
    if(position ==5){

        positionBox->addItem(QString::number(0));
        positionBox->addItem(QString::number(1));
        positionBox->addItem(QString::number(2));
        positionBox->addItem(QString::number(3));
        positionBox->addItem(QString::number(4));


    }


    if (playerQuery.exec()) {
        while (playerQuery.next()) {
            if (playerQuery.value(1).toInt() == position) {
                playerBox->addItem(playerQuery.value(0).toString());
                positionBox->addItem(QString::number(position));
            }
            else if(position ==5){
                playerBox->addItem(playerQuery.value(0).toString());



            }
        }
    }
}

void MainWindow::submitSquad(const QString& sessionId) {
   // Assuming default connection


    if (!db.transaction()) {
        QMessageBox::critical(this, "Error", "Failed to start database transaction: " );
        return;
    }
    int maxSquadId = 0;
    QSqlQuery maxIdQuery("SELECT MAX(squad_id) FROM sessionsquad");
    if (maxIdQuery.next()) {
        maxSquadId = maxIdQuery.value(0).toInt();
    }

    try {
        for (int i = 0; i < 6; ++i) {

            if (playerComboBoxes[i]->currentIndex() == -1 || positionComboBoxes[i]->currentIndex() == -1) {
                QMessageBox::warning(this, "Invalid Data", "Please make sure all players and positions are selected.");
                db.rollback();
                return;
            }
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO sessionsquad (squad_id,session_id, played_player_username, position_id) VALUES (:squad_id,:session_id, :username, :position_id)");
            insertQuery.bindValue(":session_id", sessionId);
            insertQuery.bindValue(":squad_id", maxSquadId + i + 1);
            insertQuery.bindValue(":username", playerComboBoxes[i]->currentText());
            insertQuery.bindValue(":position_id", positionComboBoxes[i]->currentText());

            if (!insertQuery.exec()) {
                throw std::runtime_error("Failed to insert player into squad: " );
            }
        }

        if (!db.commit()) {
            throw std::runtime_error("Failed to commit transaction: " );
        }
        QMessageBox::information(this, "Success", "Squad submitted successfully.");
    } catch (const std::runtime_error& e) {
        db.rollback();
        QMessageBox::critical(this, "Transaction Error", QString::fromStdString(e.what()));
    }
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
void MainWindow::addJury(const QString& username, const QString& password, const QString& name, const QString& surname, const QString& userType,  const QString& nationality) {
    if (!db.transaction()) {
        QMessageBox::critical(this, "Database Error", "Failed to start transaction");
        return;
    }

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO users (username, password, name, surname, user_type,  nationality)
        VALUES (:username, :password, :name, :surname, :userType, :nationality)
    )");

    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":name", name);
    query.bindValue(":surname", surname);
    query.bindValue(":userType", userType);


    query.bindValue(":nationality", nationality);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to insert jury: " );
        db.rollback();
        return;
    }

    db.commit();
    QMessageBox::information(this, "Success", "Jury added successfully");
}

void MainWindow::setupManagerPage() {
    QFont inputFont("Arial", 12);
    QString inputStyle = "QLineEdit {"
                         "   border: 2px solid #2F80ED;"
                         "   border-radius: 5px;"
                         "   padding: 5px;"
                         "}";

    // Adding a label for section heading
    QLabel* addJuryLabel = new QLabel("Add New Jury:");
    addJuryLabel->setFont(QFont("Arial", 14, QFont::Bold));
    managerLayout->addWidget(addJuryLabel);

    // Input fields for the jury's details
    QLineEdit* usernameInput = new QLineEdit();
    usernameInput->setPlaceholderText("Enter jury username");
    usernameInput->setFont(inputFont);
    usernameInput->setStyleSheet(inputStyle);
    managerLayout->addWidget(usernameInput);

    QLineEdit* passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Enter jury password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setFont(inputFont);
    passwordInput->setStyleSheet(inputStyle);
    managerLayout->addWidget(passwordInput);

    QLineEdit* nameInput = new QLineEdit();
    nameInput->setPlaceholderText("Enter jury name");
    nameInput->setFont(inputFont);
    nameInput->setStyleSheet(inputStyle);
    managerLayout->addWidget(nameInput);

    QLineEdit* surnameInput = new QLineEdit();
    surnameInput->setPlaceholderText("Enter jury surname");
    surnameInput->setFont(inputFont);
    surnameInput->setStyleSheet(inputStyle);
    managerLayout->addWidget(surnameInput);

    QLineEdit* nationalityInput = new QLineEdit();
    nationalityInput->setPlaceholderText("Enter jury nationality");
    nationalityInput->setFont(inputFont);
    nationalityInput->setStyleSheet(inputStyle);
    managerLayout->addWidget(nationalityInput);








    // Submit button for adding new jury
    QPushButton* submitJuryButton = new QPushButton("Add Jury");
    submitJuryButton->setStyleSheet(
        "QPushButton {"
        "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #56CCF2, stop:1 #2F80ED);"
        "   color: white;"
        "   border: 2px solid #2F80ED;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #2F80ED, stop:1 #56CCF2);"
        "}"
        "QPushButton:pressed {"
        "   background-color: #2F80ED;"
        "}"
        );

    managerLayout->addWidget(submitJuryButton);
    managerLayout->setSpacing(10);  // Adds spacing between widgets



    // Connect the button's clicked signal to the slot that handles jury creation
    connect(submitJuryButton, &QPushButton::clicked, [this, usernameInput, passwordInput, nameInput, surnameInput, nationalityInput]() {
        addJury(usernameInput->text(), passwordInput->text(), nameInput->text(), surnameInput->text(), "jury",  nationalityInput->text());
    });
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
        welcomeLabel->setFont(QFont("Arial", 14, QFont::Bold));
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
            welcomeLabel->setFont(QFont("Arial", 14, QFont::Bold));
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
            welcomeLabel->setFont(QFont("Arial", 14, QFont::Bold));
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
            welcomeLabel->setFont(QFont("Arial", 14, QFont::Bold));
            playerLayout->addWidget(welcomeLabel);
            logoutButton4 = new QPushButton("Log out");
            playerLayout->addWidget(logoutButton4);
            setupPlayerPage();
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
