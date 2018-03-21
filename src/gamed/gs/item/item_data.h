#ifndef __GAMED_GS_ITEM_ITEM_DATA_H__
#define __GAMED_GS_ITEM_ITEM_DATA_H__

#include <stdint.h>
#include <stddef.h>
#include <string>

namespace gamed {

struct itemdata 
{
	int32_t id;				//ģ��ID
	int16_t index;          //��Ʒ��λ
	int32_t count;			//��Ʒ����
	int32_t pile_limit;		//�ѵ�����
	int32_t proc_type;		//����ʽ
	int32_t recycle_price;	//�����(���ռ۸�)
	int32_t expire_date;	//�������ڣ�<=0��ʾ����ʧЧ
	int16_t item_cls;       //��Ʒ����
	std::string content;    //��Ʒ��̬����

	itemdata():
		id(0),
		index(-1),
		count(0),
		pile_limit(0),
		proc_type(0),
		recycle_price(0),
		expire_date(0),
		item_cls(0)
	{
	}

	void Clone(itemdata& data) const
	{
		data = *this;
	}
};

} // namespace gamed

#endif // __GAMED_GS_ITEM_ITEM_DATA_H__
