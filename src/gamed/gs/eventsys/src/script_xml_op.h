#ifndef GAMED_GS_EVENTSYS_SRC_SCRIPT_XML_OP_H_
#define GAMED_GS_EVENTSYS_SRC_SCRIPT_XML_OP_H_

#include <string>

#include "shared/base/callback_bind.h"


namespace gamed {

class EvScriptXML
{
public:
	struct XMLInfo
	{
		int script_id;
		std::string script_type;
		int event_id;
		std::string event_type;
		std::string evbt_name;
		std::string evbt_xml_text;
	};
	typedef shared::bind::Callback<void (const XMLInfo&)> ForEachEventCallback;
	static bool ParseScriptXML(const std::string& xml_file, const ForEachEventCallback& callback);
};

} // namespace gamed

#endif // GAMED_GS_EVENTSYS_SRC_SCRIPT_XML_OP_H_
