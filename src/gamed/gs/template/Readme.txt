一、目录说明
  1.data_templ目录是数据模板目录，里面的文件是由服务器端程序维护的，客户端Share/common/data_templ下的文件须与服务器的一致。每次修改的时候cp一份覆盖客户端data_templ下的文件就行，拷贝完毕后需要用unix2dos命令把客户端data_templ下所有文件转成windows格式。
  2.map_data目录是客户端、服务器端分开维护的，地编导出服务器端所使用的地图数据时，使用的是客户端Share/common/map_data_srv目录下的文件。修改方法与data_templ一致，直接cp覆盖客户端对应目录下的文件。


二、"FIXME"现存问题
  1.template/目录下几个子目录都是完成模板读、写文件的功能子模块，很有一部份代码是一致的，比如：基类baseXXX，codec，XXX_manager，这些代码现在还想不出方法来抽象，因为每个子目录里稍有不同,比如名字空间、base基类的成员、小工具如何使用等方面。因此这些代码一旦出bug则需要修改所有子目录里对应文件。
