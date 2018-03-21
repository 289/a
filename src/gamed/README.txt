gs代码编写说明：


一、gs编译篇
  gs编译顺序：
  1.shared/base, shared/logsys，shared/net，shared/lua，shared/security
  2.logsrvd/logclient
  3.common/obj_data下，(1)script-gen.sh (2)make
  4.common/protocol下，(1)script-gen.sh (2)make -f makefile_gs
  5.common/rpc下，(1)script-gen.sh (2)make -f makefile_master
  6.gamed/utility_lib 所有目录都make一下
  7.gamed/game_module 所有目录都make一下
  8.gamed下或者gs下make


二、Makefile篇
  1.目前shared/mk下的gcc.build.lib.mk不支持寻找绝对路径的.cpp文件，因此添加LIBSRCS或EXESRCS时不能用绝对路径，例如:gs/netio/Makefile


三、修改说明篇
  1.netio目录下是与link，master的IO操作，基本不需要改。与link，master的协议发送和接收处理都是在netmsg目录下。
  2.world和worldmanager的关系：world类代表地图本身（通过图、AOI等地图相关数据都在world上，AOI放在world上是因为player不能直接访问worldmanager），worldmanager代表在地图上的管理类。player身上有world的指针用于取地图的相关数据，但是player不能直接访问worldmanager。
  3.WorldObject是带位置、能收/发内部MSG的对象，Unit是能移动、能处理session的对象。
