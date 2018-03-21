#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>

namespace gamed
{

class Player
{
public:
	Player(int64_t roleid, int32_t hp, int32_t mp)
		: roleid_(roleid), hp_(hp), mp_(mp)
	{
	}

	int64_t GetRoleId() const
	{
		return roleid_;
	}

	void AddHp(int32_t inc_hp)
	{
		hp_ += inc_hp;
	}

	void Show() const
	{
		printf("roleid=%lld, hp=%d, mp=%d\n", roleid_, hp_, mp_);
	}
private:
	int64_t roleid_;
	int32_t hp_;
	int32_t mp_;
};

} // namespace gamed

#endif
