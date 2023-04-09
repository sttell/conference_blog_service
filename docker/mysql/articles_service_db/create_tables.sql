use admin;
show tables;
CREATE TABLE IF NOT EXISTS `UserData`
    (`id` INT NOT NULL AUTO_INCREMENT,
    `login` VARCHAR(256) CHARACTER SET ust8 COLLATE ustf8_unicode_ci NOT NULL,
    `first_name` VARCHAR(256) CHARACTER SET ust8 COLLATE ustf8_unicode_ci NOT NULL,
    `last_name` VARCHAR(256) CHARACTER SET ust8 COLLATE ustf8_unicode_ci NOT NULL,
    `second_name` VARCHAR(256) CHARACTER SET ust8 COLLATE ustf8_unicode_ci NULL
    `email` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
    `gender` INT NOT NULL,
    PRIMARY KEY (`id`),
    KEY `logn` (`login`),
    KEY `ln` (`last_name`),
    KEY `fn` (`first_name`)
);
DESCRIBE UserData;
INSERT INTO UserData (login, first_name, last_name, second_name, email, gender) values ('sttell', 'Биро','Илья','Олегович','st.tell@mail.ru',1);
SELECT * FROM UserData WHERE id < 10;