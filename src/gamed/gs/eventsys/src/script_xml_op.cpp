#include "script_xml_op.h"

#include <sstream>

#include "common/3rd/pugixml/inc/pugixml.hpp"
#include "gs/global/dbgprt.h"


namespace gamed {

using namespace pugi;

///
/// static member func
///
bool EvScriptXML::ParseScriptXML(const std::string& xml_file, const ForEachEventCallback& callback)
{
	xml_document doc;
	xml_parse_result result = doc.load_file(xml_file.c_str());
	if (!result)
	{
		__PRINTF("event BT-XML load_file error! %s", xml_file.c_str());
		return false;
	}

	XMLInfo info;
	xml_node scriptNode = doc.child("Script");
	info.script_id      = scriptNode.attribute("Id").as_int();
	info.script_type    = scriptNode.attribute("Type").value();

	// foreach event
	xml_node eventNode  = scriptNode.child("Event");
	for (; eventNode; eventNode = eventNode.next_sibling("Event"))
	{
		info.event_id   = eventNode.attribute("Id").as_int();
		info.event_type = eventNode.attribute("Type").value();
		// parse BehaviorTree xml text
		xml_node btNode = eventNode.child("BehaviorTree");
		info.evbt_name  = btNode.attribute("Name").value();
		std::ostringstream tmpos;
		btNode.print(tmpos);
		info.evbt_xml_text = tmpos.str();

		callback(info);
	}

	return true;
}

} // namespace gamed
