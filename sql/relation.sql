CREATE TABLE relation (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    question_id INT NOT NULL,
    completed BOOLEAN DEFAULT FALSE,
    favorited BOOLEAN DEFAULT FALSE,
    
    `delete_flag` TINYINT  DEFAULT 0 COMMENT '0-正常, 1-删除',
    `create_time` DATETIME DEFAULT now(),
    `update_time` DATETIME DEFAULT now()
);
