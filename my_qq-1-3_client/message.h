#ifndef MESSAGE_H
#define MESSAGE_H
#define LOGIN 1       // 登录消息
#define CHAT 2        // 普通聊天消息
#define LOGOUT 3      // 退出消息
#define REGISTER 4    // 注册信息
#define LOOKUP 5      // 查询历史消息
#define CREATEGROUP 6 // 创建群聊
#define JOINGROUP 7 //加入群聊

struct Message {
    int msg_type; // 消息类型：LOGIN、CHAT、LOGOUT
    char username[32]; // 发送者用户名
    char password[32]; // 用户密码
    char data[256]; // 消息内容
    char group_name[32]; // 群名 = 群号
    int user_socket; // 用户对应的 socket 连接
    bool login_flag; // 登录成功标志
    int group_count; // 群组数量
    char group_names[10][32]; // 存储当前用户所在的群组名称，最多 10 个
    int history_count; // 历史消息条数
    char history_messages[100][512]; // 存储历史消息，每条最多 512 字节
};
enum MessageType {
    MSG_LOGIN           = 1,  // 登录
    MSG_CHAT            = 2,  // 群聊消息
    MSG_LOGOUT          = 3,  // 退出
    MSG_REGISTER        = 4,  // 注册
    MSG_LOOKUP          = 5,  // 查询历史消息请求
    MSG_CREATE_GROUP    = 6,  // 创建群聊
    MSG_JOIN_GROUP      = 7,  // 加入群聊
    MSG_LOOKUP_RESPONSE = 8,  // 查询历史消息响应
    MSG_LOGIN_RES       = 9
};

// 消息头
typedef struct {
    MessageType type;      // 消息类型
    char sender[64];       // 发送者用户名
    char receiver[64];     // 接收者或群组名
} MessageHeader;

// 注册 / 登录 Payload
typedef struct {
    char username[64];
    char password[64];
} AuthPayload;

// 群聊消息 Payload
typedef struct {
    char content[512];     // 文本内容
} ChatPayload;

// 查询历史记录请求 Payload
typedef struct {
    char groupName[64];    // 要查询的群组名
    int  maxCount;         // 最大返回条数
} LookupRequestPayload;

// 查询历史记录响应 Payload
#define MAX_HISTORY_ITEMS 100
typedef struct {
    int  count;                          // 实际返回条数
    char messages[MAX_HISTORY_ITEMS][512]; // 历史消息数组
} LookupResponsePayload;

// 群组管理 Payload
typedef struct {
    char groupName[64];    // 群组名
} GroupPayload;
// 登录响应 Payload
#define MAX_GROUPS 10
typedef struct {
    bool loginSuccess;           // 登录是否成功
    int  groupCount;             // 已加入群组数量
    char groupNames[MAX_GROUPS][64];  // 已加入群组名称列表
} LoginResponsePayload;
// 完整消息结构体
typedef struct {
    MessageHeader header;
    union {
        AuthPayload           auth;        // MSG_LOGIN, MSG_REGISTER
        ChatPayload           chat;        // MSG_CHAT
        LookupRequestPayload  lookupReq;   // MSG_LOOKUP
        LookupResponsePayload lookupRsp;   // MSG_LOOKUP_RESPONSE
        GroupPayload          group;       // MSG_CREATE_GROUP, MSG_JOIN_GROUP
        LoginResponsePayload loginResp;   //
    } payload;
} Message2;
#endif // MESSAGE_H
