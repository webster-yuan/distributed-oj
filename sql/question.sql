CREATE TABLE IF NOT EXISTS questions (
	`id` int PRIMARY KEY AUTO_INCREMENT COMMENT '题目的ID',
	`title` VARCHAR(64) NOT NULL COMMENT '题目的标题',
	`star` VARCHAR(8) NOT NULL COMMENT '题目的难度',
	`question_desc` TEXT NOT NULL COMMENT '题目描述',
	`header` TEXT NOT NULL COMMENT '题目头部，给用户看的代码',
	`tail` TEXT NOT NULL COMMENT '题目尾部，包含我们的测试用例',
	`time_limit` int DEFAULT 1 COMMENT '题目的时间限制',
	`mem_limit` int DEFAULT 5000000 COMMENT '题目的空间限制',
    `delete_flag` TINYINT  DEFAULT 0 COMMENT '0-正常, 1-删除',
    `create_time` DATETIME DEFAULT now(),
    `update_time` DATETIME DEFAULT now()
)ENGINE=INNODB DEFAULT CHARSET=utf8;