std_msgs/Header header   # 时间戳和坐标系ID

string[] name            # 关节名称列表
float64[] position       # 关节位置（旋转关节为弧度，平移关节为米）
float64[] velocity       # 关节速度（弧度/秒 或 米/秒）
float64[] effort         # 关节力矩或力（Nm 或 N）