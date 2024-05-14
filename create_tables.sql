CREATE TABLE database_managers (
    username VARCHAR(250) PRIMARY KEY,
    password VARCHAR(250) NOT NULL
);

CREATE TABLE coaches (
    username VARCHAR(250) PRIMARY KEY NOT NULL,
    password VARCHAR(250) NOT NULL,
    name VARCHAR(250) NOT NULL,
    surname VARCHAR(250) NOT NULL,
    nationality VARCHAR(250) NOT NULL
);

CREATE TABLE jury (
    username VARCHAR(250) PRIMARY KEY NOT NULL,
    password VARCHAR(250) NOT NULL,
    name VARCHAR(250) NOT NULL,
    surname VARCHAR(250) NOT NULL,
    nationality VARCHAR(250) NOT NULL
);

CREATE TABLE players (
    username VARCHAR(250) PRIMARY KEY NOT NULL,
    password VARCHAR(250) NOT NULL,
    name VARCHAR(250) NOT NULL,
    surname VARCHAR(250) NOT NULL,
    date_of_birth DATE NOT NULL,
    height DECIMAL(5,2) NOT NULL,
    weight DECIMAL(5,2) NOT NULL
);


CREATE TABLE stadiums (
    stadium_id INT PRIMARY KEY NOT NULL,
    stadium_name VARCHAR(250) NOT NULL,
    stadium_country VARCHAR(250) NOT NULL
);

CREATE TABLE positions (
    position_id INT PRIMARY KEY,
    position_name VARCHAR(250) NOT NULL
);


CREATE TABLE tvchannels (
    channel_id INT PRIMARY KEY,
    channel_name VARCHAR(250) NOT NULL
);


CREATE TABLE teams (
    team_id INT PRIMARY KEY,
    team_name VARCHAR(250) NOT NULL,
    coach_username VARCHAR(250) NOT NULL,
    contract_start DATE NOT NULL,
    contract_finish DATE NOT NULL,
    channel_id INT NOT NULL,
    CONSTRAINT teams_chk_1 CHECK (contract_start <= contract_finish),
    FOREIGN KEY (coach_username) REFERENCES coaches(username),
    FOREIGN KEY (channel_id) REFERENCES tvchannels(channel_id)
);

CREATE TABLE playerpositions(
	player_positions_id INT PRIMARY KEY,
    username VARCHAR(250) NOT NULL,
    position_id INT NOT NULL,
    FOREIGN KEY (username) REFERENCES players(username),
    FOREIGN KEY (position_id) REFERENCES positions(position_id)
);

CREATE TABLE playerteams(
	player_teams_id INT PRIMARY KEY,
    username VARCHAR(250) NOT NULL,
    team_id INT NOT NULL,
    FOREIGN KEY (username) REFERENCES players(username),
	FOREIGN KEY (team_id) REFERENCES teams(team_id)
);

CREATE TABLE matchsessions(
	session_id INT PRIMARY KEY,
    team_id INT NOT NULL,
    time_slot ENUM('1', '2', '3', '4') NOT NULL,
    date DATE NOT NULL,
    assigned_jury_username VARCHAR(255) NOT NULL,
    rating DECIMAL(3,1),
    stadium_id INT NOT NULL ,
    FOREIGN KEY (stadium_id) REFERENCES stadiums(stadium_id),
	FOREIGN KEY (team_id) REFERENCES teams(team_id),
    FOREIGN KEY (assigned_jury_username) REFERENCES jury(username),
	UNIQUE INDEX idx_match (stadium_id, date, time_slot)
);

CREATE TABLE sessionsquad(
    squad_id INT ,
    session_id INT NOT NULL,
    played_player_username VARCHAR(255) NOT NULL,
    position_id INT NOT NULL,
    PRIMARY KEY (squad_id),
    FOREIGN KEY (session_id) REFERENCES MatchSessions(session_id) ON DELETE CASCADE,
    FOREIGN KEY (played_player_username) REFERENCES players(username),
    FOREIGN KEY (position_id) REFERENCES Positions(position_id)
);