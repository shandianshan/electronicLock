##### 需求：
用户初始密码123456，常态为红灯

按open键触发开锁操作，按#键结束输入
密码输入正确：锁打开（变绿灯）    10秒后变红灯
密码输入错误不反应，连续三次错误锁定5秒，期间黄灯闪烁（直接闪5秒）


密码输入用\*表示

修改密码：modify secret 键触发修改密码操作。首先进入认证模块，认证成功后设置新密码。认证失败则恢复初始状态

输入admin-key触发管理员功能，系统提示输入管理  管理员密码8位数字，内置。认证成功自动恢复默认密码123456

组合键功能：拓展可出现的字符，从0-9增加10个字母

delete键触发删除操作，重新输入密码。

##### 业务流程

初始：接受码，若码为open或admin-key进入相应模块

##### 规则
- 密码一定为6位
- 输入密码过程中可输入非法字符，输入del后立即重新输入。多于6个字符后程序仅对#和del反应


![[Pasted image 20230317192156.png]]
![[Pasted image 20230317192405.png]]

##### 主要代码
- electronicLock.c 
	主要业务逻辑
	对数码管的控制
	对红绿黄灯的控制
	对键盘的驱动
- screen.c
	对液晶屏的控制
- screen.h 
	暴露两个API。分别用于显示任意英文字母字符串和任意数量的 ‘\*\'
	