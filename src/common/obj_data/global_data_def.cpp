#include "global_data_def.h"


namespace common {

// init static members
INIT_STAITC_OBJECTATTR(InstanceRecordData, GDT_INS_RECORD_DATA);
INIT_STAITC_OBJECTATTR(WorldBossRecordData, GDT_WB_RECORD_DATA);
INIT_STAITC_OBJECTATTR(WorldBossInfoData, GDT_WB_INFO_DATA);

///
/// GlobalDataDeclare
///
bool GlobalDataDeclare::Marshal(ObjectAttrPacket& packet)
{
	return ObjAttrPacketManager::MarshalPacket(packet);
}

} // namespace common

