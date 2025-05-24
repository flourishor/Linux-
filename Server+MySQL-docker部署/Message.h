//
// Created by f on 25-5-14.
//

#ifndef MESSAGE_H
#define MESSAGE_H

// 消息类型枚举
enum MessageType {
    MSG_LOGIN = 1, // 登录
    MSG_CHAT = 2, // 群聊消息
    MSG_LOGOUT = 3, // 退出
    MSG_REGISTER = 4, // 注册
    MSG_LOOKUP = 5, // 查询历史消息请求
    MSG_CREATE_GROUP = 6, // 创建群聊
    MSG_JOIN_GROUP = 7, // 加入群聊
    MSG_LOOKUP_RESPONSE = 8, // 查询历史消息响应
    MSG_LOGIN_RES = 9 // 登陆返回信息
};

// 消息头
typedef struct {
    MessageType type; // 消息类型
    char sender[64]; // 发送者用户名
    char receiver[64]; // 接收者或群组名
} MessageHeader;

// 注册 / 登录 Payload
typedef struct {
    char username[64];
    char password[64];
} AuthPayload;

// 群聊消息 Payload
typedef struct {
    char content[512]; // 文本内容
} ChatPayload;

// 查询历史记录请求 Payload
typedef struct {
    char groupName[64]; // 要查询的群组名
    int maxCount; // 最大返回条数
} LookupRequestPayload;

// 查询历史记录响应 Payload
#define MAX_HISTORY_ITEMS 100

typedef struct {
    int count; // 实际返回条数
    char messages[MAX_HISTORY_ITEMS][512]; // 历史消息数组
} LookupResponsePayload;

// 群组管理 Payload
typedef struct {
    char groupName[64]; // 群组名
} GroupPayload;

// 登录响应 Payload
#define MAX_GROUPS 10

typedef struct {
    bool loginSuccess; // 登录是否成功
    int groupCount; // 已加入群组数量
    char groupNames[MAX_GROUPS][64]; // 已加入群组名称列表
} LoginResponsePayload;

// 完整消息结构体
typedef struct {
    MessageHeader header;

    union {
        AuthPayload auth; // MSG_LOGIN, MSG_REGISTER
        ChatPayload chat; // MSG_CHAT
        LookupRequestPayload lookupReq; // MSG_LOOKUP
        LookupResponsePayload lookupRsp; // MSG_LOOKUP_RESPONSE
        GroupPayload group; // MSG_CREATE_GROUP, MSG_JOIN_GROUP
        LoginResponsePayload loginResp; //
    } payload;
} Message;

#endif //MESSAGE_H
