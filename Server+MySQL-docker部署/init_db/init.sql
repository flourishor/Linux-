CREATE DATABASE IF NOT EXISTS qq_db_1_3;
USE qq_db_1_3;

-- 创建 users 表
CREATE TABLE IF NOT EXISTS users (
    uid INT NOT NULL AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (uid)
);

-- 创建 qq_groups 表
CREATE TABLE IF NOT EXISTS qq_groups (
    group_id INT NOT NULL AUTO_INCREMENT,
    group_name VARCHAR(64) NOT NULL UNIQUE,
    owner_id INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (group_id),
    FOREIGN KEY (owner_id) REFERENCES users(uid) ON DELETE CASCADE
);

-- 创建 group_members 表
CREATE TABLE IF NOT EXISTS group_members (
    id INT NOT NULL AUTO_INCREMENT,
    group_id INT NOT NULL,
    uid INT NOT NULL,
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_admin TINYINT(1) DEFAULT 0,
    PRIMARY KEY (id),
    FOREIGN KEY (group_id) REFERENCES qq_groups(group_id) ON DELETE CASCADE,
    FOREIGN KEY (uid) REFERENCES users(uid) ON DELETE CASCADE
);
-- 创建 group_messages 表
CREATE TABLE IF NOT EXISTS group_messages (
    msg_id     INT PRIMARY KEY AUTO_INCREMENT COMMENT '消息ID',
    group_id   INT NOT NULL COMMENT '群组ID',
    sender_id  INT NOT NULL COMMENT '发送者ID',
    message    TEXT NOT NULL COMMENT '消息内容',
    sent_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '发送时间',
    FOREIGN KEY (group_id) REFERENCES qq_groups(group_id) ON DELETE CASCADE,
    FOREIGN KEY (sender_id) REFERENCES users(uid) ON DELETE CASCADE
);
-- SELECT users.username,group_messages.message,group_messages.sent_at FROM users JOIN group_messages ON users.uid = group_messages.sender_id JOIN qq_groups ON group_messages.group_id = qq_groups.group_id where qq_groups.group_name = "avangers" ;
-- INSERT INTO qq_groups (group_name, owner_id)
-- VALUES ("brothers", (SELECT uid FROM users WHERE username = "12345"));

-- INSERT INTO group_members (group_id, uid, is_admin)
-- VALUES (
--   (SELECT group_id FROM qq_groups WHERE group_name = "brothers" AND owner_id = (SELECT uid FROM users WHERE username = "12345")),
--   (SELECT uid FROM users WHERE username = "12345"),
--   1
-- );

-- insert into group_members (group_id, uid, is_admin)
-- values (
--     (select group_id from qq_groups where group_name = '%s'),
--     (select uid from users where username = '%s'),
--     1
-- );
-- insert into group_messages (group_id,sender_id,message)
-- values(
--     (select group_id from qq_groups where group_name = '%s'),
--     (select uid from users where username = '%s'),
--     '%s'
-- );
