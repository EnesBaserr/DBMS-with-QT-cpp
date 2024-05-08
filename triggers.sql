


DELIMITER $$

CREATE TRIGGER PreventOverlappingSessions
BEFORE INSERT ON MatchSessions
FOR EACH ROW
BEGIN
    -- Check for overlapping sessions at the same stadium and time slots
    DECLARE conflict_count INT;
    SELECT COUNT(*) INTO conflict_count
    FROM MatchSessions AS ms
    JOIN stadiums AS s ON ms.stadium_id = s.stadium_id
    WHERE s.stadium_name = (SELECT stadium_name FROM stadiums WHERE stadium_id = NEW.stadium_id)
      AND NEW.date = ms.date
      AND (
        NEW.time_slot = ms.time_slot OR
        NEW.time_slot + 1 = ms.time_slot OR
        NEW.time_slot - 1 = ms.time_slot
      );

    -- If there is an overlap, prevent the insertion
    IF conflict_count > 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Cannot insert the match session due to overlapping time slots at the same stadium.';
    END IF;


END$$

DELIMITER ;


DELIMITER $$

CREATE TRIGGER CheckPlayerTimeOverlap
BEFORE INSERT ON sessionsquad
FOR EACH ROW
BEGIN
    DECLARE player_exists INT;

    SELECT COUNT(*) INTO player_exists
    FROM sessionsquad ss
    JOIN matchsessions ms ON ss.session_id = ms.session_id
    JOIN matchsessions newms ON newms.session_id = NEW.session_id
    WHERE ss.played_player_username = NEW.played_player_username
    AND newms.date = ms.date
    AND (ms.time_slot = newms.time_slot + 1 OR ms.time_slot = newms.time_slot - 1 OR ms.time_slot=newms.time_slot);

    IF player_exists > 0 THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'A player cannot be in multiple sessions on the same date within adjacent and same time slots.';
    END IF;
END$$

DELIMITER ;


DELIMITER $$
CREATE TRIGGER CheckPlayerRegisteredPosition
BEFORE INSERT ON sessionsquad
FOR EACH ROW
BEGIN
    DECLARE position_registered INT;

    -- Check if the player is registered for the position they are being inserted into
    SELECT COUNT(*) INTO position_registered
    FROM PlayerPositions
    WHERE username = NEW.played_player_username AND position_id = NEW.position_id;

    -- If the player is not registered for this position, prevent the insertion
    IF position_registered = 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Player cannot play in a position they are not registered for.';
    END IF;
END$$

DELIMITER;

DELIMITER $$
CREATE TRIGGER PreventDuplicatePlayerSession
BEFORE INSERT ON sessionsquad
FOR EACH ROW
BEGIN
    DECLARE player_count INT;

    -- Check if the player is already registered for the same session
    SELECT COUNT(*) INTO player_count
    FROM SessionSquad
    WHERE session_id = NEW.session_id AND played_player_username = NEW.played_player_username;

    -- If the player is already registered for this session, prevent the insertion
    IF player_count > 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'A player cannot be registered more than once in the same session.';
    END IF;
END$$

DELIMITER;

DELIMITER $$

CREATE TRIGGER PreventDuplicateRating
BEFORE INSERT ON MatchSessions
FOR EACH ROW
BEGIN
    DECLARE existing_rating_count INT;
    SELECT COUNT(*) INTO existing_rating_count
    FROM MatchSessions
    WHERE session_id = NEW.session_id AND assigned_jury_username = NEW.assigned_jury_username;

    IF existing_rating_count > 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'A jury can rate each match session only once.';
    END IF;
END$$

DELIMITER ;


DELIMITER $$

CREATE TRIGGER PreventRatingEdit
BEFORE UPDATE ON MatchSessions
FOR EACH ROW
BEGIN
    IF OLD.rating IS NOT NULL AND NEW.rating <> OLD.rating THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Ratings cannot be edited once submitted.';
    END IF;
END$$

DELIMITER ;






DELIMITER $$

CREATE TRIGGER CheckPlayerRegisteredPosition
BEFORE INSERT ON SessionSquad
FOR EACH ROW
BEGIN
    DECLARE position_registered INT;

    -- Check if the player is registered for the position they are being inserted into
    SELECT COUNT(*) INTO position_registered
    FROM PlayerPositions
    WHERE username = NEW.played_player_username AND position_id = NEW.position_id;

    -- If the player is not registered for this position, prevent the insertion
    IF position_registered = 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Player cannot play in a position they are not registered for.';
    END IF;


END$$

DELIMITER ;
