# UR5-Drawing

首先感谢实验课的助教工作，然我看到了学生们的热情以及大家的智慧。这是我在其中绘图机器人实验的基础上进行复现完成的一次demo演示。

基于ROS，我们利用gcode与moveit结合产生了一种离线生成路点，并利用笛卡尔路径规划进行运动学规划的绘图方式。


# 本系统在 Ubuntu 16.04 + ros kinetic下测试通过 ，机械臂控制采用moveit  
 

### 整体方案：
图片转换成svg格式->svg图片边缘轮廓->转为gcode->解析gcode ->运动学逆解执行->笛卡尔路径规划->机械臂空间绘图

 

### 具体原理：

# Blog：

`universal_robot`



### 文件结构
`universal_robot` UR机械臂驱动 

`industrial_core` UR机械臂驱动依赖 

`draw_core` 绘图部分核心代码

- `arm_controller.py` 机械臂控制核心
- `gcode_draw.py` gcode解析与绘制
- `gcode_excute` gcode指令与机械臂控制的中间接口文件
- `svg_convert` svg转gcode
- `py_svg2gcode` svg转Gcode核心代码库
  - `lib/shapes.py` svg形状类型解析
  - `lib/cubicsuperpath.py` 曲线插值

## 参考网址：

https://github.com/mywisdomfly/drawing_manipulator
