#ifndef SHARED_NET_PACKET_CMD_DEFINE_H_
#define SHARED_NET_PACKET_CMD_DEFINE_H_

///
/// cmd is the interaction with the client protocol packet number.
/// cmd number: segment allocation as follow
///
/// 下面的命令号段是不能重叠的（同一方向，比如:C2X，或者X2C）
///
//- client to link
#define C2L_CMD_LOWER_LIMIT ((int)100)                          // 100
#define C2L_CMD_UPPER_LIMIT (int)(C2L_CMD_LOWER_LIMIT + 800)    // 900
//- client to master
#define C2M_CMD_LOWER_LIMIT (int)(C2L_CMD_UPPER_LIMIT + 100)    // 1000
#define C2M_CMD_UPPER_LIMIT (int)(C2M_CMD_LOWER_LIMIT + 900)    // 1900
//- client to gameserver
#define C2G_CMD_LOWER_LIMIT (int)(C2M_CMD_UPPER_LIMIT + 100)    // 2000
#define C2G_CMD_UPPER_LIMIT (int)(C2G_CMD_LOWER_LIMIT + 7900)   // 9900
//- client to gateway
#define C2W_CMD_LOWER_LIMIT (int)(C2G_CMD_UPPER_LIMIT + 100)	// 10000
#define C2W_CMD_UPPER_LIMIT (int)(C2W_CMD_LOWER_LIMIT + 800)	// 10800
//- client to auth
#define C2A_CMD_LOWER_LIMIT (int)(C2W_CMD_UPPER_LIMIT + 100)	// 10900
#define C2A_CMD_UPPER_LIMIT (int)(C2A_CMD_LOWER_LIMIT + 800)	// 11700
//- client to client
#define C2C_CMD_LOWER_LIMIT (int)(C2A_CMD_UPPER_LIMIT + 100)    // 11800
#define C2C_CMD_UPPER_LIMIT (int)(C2C_CMD_LOWER_LIMIT + 500)    // 12300
//- link to client
#define L2C_CMD_LOWER_LIMIT (int)(C2L_CMD_LOWER_LIMIT)
#define L2C_CMD_UPPER_LIMIT (int)(C2L_CMD_UPPER_LIMIT)
//- master to client
#define M2C_CMD_LOWER_LIMIT (int)(C2M_CMD_LOWER_LIMIT)
#define M2C_CMD_UPPER_LIMIT (int)(C2M_CMD_UPPER_LIMIT)
//- gameserver to client
#define G2C_CMD_LOWER_LIMIT (int)(C2G_CMD_LOWER_LIMIT)
#define G2C_CMD_UPPER_LIMIT (int)(C2G_CMD_UPPER_LIMIT)
//- gateway to client
#define W2C_CMD_LOWER_LIMIT (int)(C2W_CMD_LOWER_LIMIT)
#define W2C_CMD_UPPER_LIMIT (int)(C2W_CMD_UPPER_LIMIT)
//- auth to client
#define A2C_CMD_LOWER_LIMIT (int)(C2A_CMD_LOWER_LIMIT)
#define A2C_CMD_UPPER_LIMIT (int)(C2A_CMD_UPPER_LIMIT)
///
/// Debug cmd
///
#define DEBUG_CMD_LOWER_LIMIT ((int)20000)
#define DEBUG_CMD_UPPER_LIMIT ((int)30000)
//- client to link
#define C2L_DEBUG_CMD_LOWER_LIMIT (int)(DEBUG_CMD_LOWER_LIMIT)              // 20000
#define C2L_DEBUG_CMD_UPPER_LIMIT (int)(C2L_DEBUG_CMD_LOWER_LIMIT + 500)    // 20500
//- client to master
#define C2M_DEBUG_CMD_LOWER_LIMIT (int)(C2L_DEBUG_CMD_UPPER_LIMIT + 100)    // 20600
#define C2M_DEBUG_CMD_UPPER_LIMIT (int)(C2M_DEBUG_CMD_LOWER_LIMIT + 1000)   // 21600
//- client to gameserver
#define C2G_DEBUG_CMD_LOWER_LIMIT (int)(C2M_DEBUG_CMD_UPPER_LIMIT + 100)    // 21700
#define C2G_DEBUG_CMD_UPPER_LIMIT (int)(DEBUG_CMD_UPPER_LIMIT)              // 30000


#endif // SHARED_NET_PACKET_CMD_DEFINE_H_
