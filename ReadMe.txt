R0001简单功能说明：
	本设备支持14443A,M1系列卡。主要针对特定标签进行数据校验及输出控制。当场内存在M1标签，无论其合法性，设备指示灯均会闪烁，倘若标签合法，蜂鸣器会产生报警信号同时输出端口会持续输出高电平，直到标签离场500ms后停止输出。
	数据校验方式 ： keyA + Block双重加密校验方式，当密钥正确且块内数据校验通过，则此标签合法，反正不合法。
	 本设备目前测试版本，根据加密数据块内数据进行校验。
	其中默认校验区域为扇区2（后期可配）数据块0（后期可配）位置8个字节，校验密钥keyA为0x131413141314（后期可配）, 校验数据1为0x2121212121212121（后期可配）,校验数据2为0x5252525252525252（后期可配）。
	简单演示：
		1.卡A扇区2密码为0x131413141314，中块0数据为0x2121212121212121，标签入场后，指示灯闪烁，蜂鸣器报警，同时IO1输出高电平，表现为红灯常亮，标签离场，蜂鸣器和指示灯停止动作，500ms后IO1停止高电平输出，表现为红灯熄灭。
		2.卡B扇区2密码为0x131413141314，中块0数据为0x5252525252525252，标签入场后，指示灯闪烁，蜂鸣器报警，同时IO2输出高电平，表现为绿灯常亮，标签离场，蜂鸣器和指示灯停止动作，500ms后IO2停止高电平输出，表现为红灯熄灭。
		3.卡3扇区2密码为0xFFFFFFFFFFFF，块0数据为0x0000000000000000，标签入场后，指示灯闪烁，标签离场，指示灯停止动作。
		4.其他M1标签表现同3。
