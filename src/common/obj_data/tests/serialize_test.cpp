#include "common/obj_data_pool/player_data.h"
#include "shared/net/buffer.h"

using namespace shared::net;
using namespace common;

int main()
{
	PlayerData player;
	player.role_info.roleid     = 123456;
	player.base_info.gender     = 1;
	player.base_info.profession = 11;
	player.base_info.level      = 41;
	player.base_info.hp         = 100;
	player.base_info.mp         = 101;
	player.location.x           = 10.1f;
	player.location.y           = 14.4f;
	player.location.dir         = 7;
	player.location.world_id    = 1;
	player.location.world_tag   = 11;

	Buffer buf;
	player.Serialize(&buf);

	PlayerData player2;
	player2.Deserialize(buf.peek(), buf.ReadableBytes());

	player2.location.x          = 5.4444;
	player2.location.y          = 6.3333;

	// 
	Buffer buf2;
	player2.Serialize(&buf2);
	PlayerData player3;
	player3.Deserialize(buf2.peek(), buf2.ReadableBytes());

	return 0;
}
