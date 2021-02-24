此处下载的universal_robot包为ros-industrial官方包,ur_modern_driver包的下载也为官方提供
在编译时候出现以下问题：
universal_robot/ur_modern_driver/src/ur_hardware_interface.cpp:186:22: error: ‘const struct hardware_interface::ControllerInfo’ has no member named ‘hardware_interface’
controller_it->hardware_interface.c_str());

解决方法是：
需要修改ur_hardware_interface.cpp
具体地改变controller_it-> hardware_interface到controller_it->类型
ur_hardware_interface.cpp文件路径为:universal_robot/ur_modern_driver/src/ur_hardware_interface.cpp

修改后的ur_hardware_interface.cpp链接如下：
https://github.com/iron-ox/ur_modern_driver/blob/883070d0b6c0c32b78bb1ca7155b8f3a1ead416c/src/ur_hardware_interface.cpp
修改过后，重新编译，即可。

此处的包为已经修改ur_hardware_interface.cpp文件过后未做其他修改的可用包。
