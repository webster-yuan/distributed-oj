CREATE TABLE question_expansion (
    id INT AUTO_INCREMENT PRIMARY KEY,
    question_id INT NOT NULL COMMENT '记录题目id',
    creator_id INT NOT NULL COMMENT '创作者id',
    completed_count INT COMMENT '题目被完成数目',
    favorited_count INT COMMENT '题目被收藏数目',
    `delete_flag` TINYINT DEFAULT 0 COMMENT '0-正常, 1-删除',
    `create_time` DATETIME DEFAULT NOW(),
    `update_time` DATETIME DEFAULT NOW(),

    int reserved_int COMMENT '保留字段INT';
    std::string reserved_string COMMENT '保留字段string';

) ENGINE=INNODB DEFAULT CHARSET=utf8;