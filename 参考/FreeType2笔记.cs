#象素数 = 点数*分辨率/72 
//pixel=fontsize(dot) / 72(dot/inch) * dpi(pixel/inch)

#象素坐标＝ 网格坐标*象素数/EM大小

#EM正方形 //通常TrueType=2048单元

#主轮廓 //存储在字体文件中的轮廓 //‘点坐标’用‘字体单元’表示

#网格对齐 //转换到位图时,通过网格对齐(grid-fitting也叫hinting)到目标设备的象素网格
	#显式网格对齐 //对小字体结果很好,慢,一致,大,编写难
	#隐式网格对齐（也叫hinting）//小字体不好（需反走样）,快,不一致,Hint小
	#自动网格对齐 //小字体不好（需反走样）,速度依赖对齐算法,不一致,小
	
#基线(baseline)、笔(pen) //基线是假想线（垂直'或'水平）,基线有一虚拟点叫笔位置
	#水平布局 //向右'或'向左
		#步进宽度(advance width) > 0
	#垂直布局 //字形在基线居中放置
	
#上行高度（ascent） //基线到放置轮廓点最高/上的网格坐标，因为Y轴向上，所以是正值 
#下行高度（descent）//基线到放置轮廓点最低/下的网格坐标，因为Y轴向上，所以是负值 
#行距（linegap）    //两行文本间必须距离，基线到基线距离应该计算成 【上行高度 - 下行高度 + 行距】

#边界框（bounding box，bbox）//假想框子以尽可能紧密装入字形，【xMin、yMin、xMax、yMax】，用于网格对齐

