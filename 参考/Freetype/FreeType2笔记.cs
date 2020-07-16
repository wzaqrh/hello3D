#象素数 = 点数*分辨率/72 
//pixel=fontsize(dot) / 72(dot/inch) * dpi(pixel/inch)
//pixel=char_width*64/72*horz_resolution //FT_Set_Char_Size

#象素坐标＝ 网格坐标*象素数/EM大小

#EM正方形 //通常TrueType=2048单元

#主轮廓 //存储在字体文件中的轮廓 //‘点坐标’用‘字体单元’表示
	#字形可以超出EM正方形

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

#内部leading //字形出了EM正方形空间数量 //internal leading = ascent – descent – EM_size
#外部leading //行距的别名

#跨距（bearing）和步进
	#左跨距（bearingX） //从笔位置到字形左bbox边界水平距离
	#上跨距（bearingY） //基线到bbox上边界垂直距离
	#步进宽度（advanceX）//字形渲染后，笔位置增加的水平距离  //步进宽度和边界框大小无关 //VA步进宽度不是V+A步进之和
	#步进高度（advanceY）//字形渲染后，笔位置减少的垂直距离
	#字形宽度 //字形水平长度 //未缩放字体坐标=bbox.xMax-bbox.xMin
	#字形高度 //字形垂直长度 //未缩放字体坐标=bbox.yMax-bbox.yMin
	#右跨距   //bbox右边到步进宽度距离,advance_width – left_side_bearing – (xMax-xMin) 


#网格对齐的效果 //hinting将字形控制点对齐到象素网格,会修改字符映象尺寸
		//映象的‘宽度’、‘高度’、‘边界框’、'跨距'、'步进'都会修改
		//简单缩放字体上行或下行高度可能不正确，可能补救:保持被缩放上行高度顶和被缩放下行高度底
		//hint范围内多个字形并步进总宽度是不可能的，要一个个步进
		//hinting依赖最终字符宽度和高度象素值，所以它依赖分辨率 //对已hint的轮廓,字形轮廓处理2D变换时，注意使用整型象素距离（否则会很难看）
	
FT_Set_Char_Size(
	face,	/* face对象的句柄 */ 
	0, 		/* 以1/64点为单位的字符宽度 */
    16*64, 	/* 以1/64点为单位的字符高度 */
    300, 	/* 设备水平分辨率 */
    300 ); 	/* 设备垂直分辨率 */

glyph_index = FT_Get_Char_Index(face, charcode);

FT_Load_Glyph(
    face, 			/* face对象的句柄 */
    glyph_index, 	/* 字形索引 */
    load_flags ); 	/* 装载标志，参考下面  default=FT_LOAD_DEFAULT*/
//'嵌入位图'优先于'原生图像格式'
//visit by face->glyph

//若face->glyph->format != FT_GLYPH_FORMAT_BITMAP，可通过FT_Render_Glyph直接转换为位图
FT_Render_Glyph(face->glyph, render_mode);//FT_RENDER_MODE_NORMAL=抗锯齿(256级灰度)
//通过glyph->bitmap直接访问，用glyph->bitmap_left和glyph->bitmap_top指定起始位置





















	