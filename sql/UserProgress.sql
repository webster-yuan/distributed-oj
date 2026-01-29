CREATE TABLE IF NOT EXISTS `UserProgress` (
    `progress_id` INT AUTO_INCREMENT PRIMARY KEY,
    `user_id` INT NOT NULL,
    `question_id` INT NOT NULL,
    `is_completed` BOOLEAN DEFAULT FALSE,
    `completed_time` TIMESTAMP NULL,
    `attempt_count` INT DEFAULT 0,
    FOREIGN KEY (`user_id`) REFERENCES `User`(`userid`) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (`question_id`) REFERENCES `questions`(`id`) ON DELETE CASCADE ON UPDATE CASCADE,
    UNIQUE (`user_id`, `question_id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;
