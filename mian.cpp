#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <time.h>
#include <math.h>
#include "tools.h"
#include "vector2.h"

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib");//导入库，可以播放音效


#define WIN_WIDTH 900
#define WIN_HEIGTH 600

int all_zm_num = 20;

enum {WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT};

IMAGE imgBg;//表示背景图片
IMAGE imgBar;//工具栏
IMAGE imgCard[ZHI_WU_COUNT];  //植物卡牌
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];  //放置植物逐帧卡片
IMAGE imgSunshineBall[29];  //阳光球的逐帧动画
IMAGE imgZM_Walk[22];  //僵尸行走的逐帧动画
IMAGE imgZM_Dead[20];  //僵尸死亡的逐帧动画
IMAGE imgZM_Eat[21];  //僵尸吃植物的逐帧动画
IMAGE imgBulletNormal; //正常状态下子弹的卡片
IMAGE imgBulletBlast[4];//子弹爆炸
IMAGE game_win;

int sunshineNum; //阳光值
int curZhiWu; //0表示没有选中；1表示选中第一种植物
int curX, curY;//当前移动过程中植物的坐标

struct zhiwu
{
	int x, y;
	int type;//表示第几种植物
	int frameIndex; //序列帧的序号
	
	bool catched;//是否被僵尸捕获
	int blood; //植物血量
	int bloodTimer; //血条计时，超过一定次数则减血量

	int sunshineTimer; //记录向日葵产生阳光球的时间间隔

	//下面为攻击性植物的属性
	int timer;//计时器，记录距离上次发射子弹的时间间隔
};

struct zhiwu map[3][9];//地图上植物分布


//阳光球，每个阳光球25个阳光值
struct sunshineBall
{
	int x; //当前坐标
	int y;

	int targetY;//目标y位置，垂直降落
	
	int frameIndex; //动画帧索引

	bool used;//判断该阳光球在阳光池中是否使用

	int timer;//计时器，从掉落至草地开始计时，过了500帧之后还没被捡起，则作废

	bool leap;//是否处于飞跃状态
	double xoff; //飞跃过程中x和y坐标的偏移量
	double yoff;
};


//阳光池
struct sunshineBall balls[10];

//僵尸
struct zm
{
	int x, y;//y = row*100 + 180
	int row;//僵尸走第几行
	int frameIndex;
	bool used;
	int speed;
	int blood;//僵尸血量
	int harm; //僵尸对植物的伤害值
	bool dead;//判断是否死亡
	bool eating; //判断是否正在吃植物
};

//僵尸池
struct zm zms[10];

//子弹
struct bullet
{
	int x, y;
	int row;
	bool used;
	int speed;
	int harm;

	bool balst;//是否处于爆炸状态
	int frameIndex;//爆炸动画帧数
	int timer;//计时器
};

//子弹池
struct bullet bullets[30];

//判断文件是否存在
bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		return false;
	}

	fclose(fp);
	return true;
}


//游戏初始化
void gameInit()
{
	//把字符集修改为“多字节字符集”

	//初始化阳光值
	sunshineNum = 100;

	//加载游戏背景图片
	loadimage(&imgBg, "res/bg.jpg");

	//加载工具栏图片
	loadimage(&imgBar, "res/bar5.png");

	//加载植物动画,在下面for循环里
	memset(imgZhiWu, 0, sizeof(imgZhiWu));

	//加载胜利动画
	loadimage(&game_win, "res/gameWin.png");

	//地图内植物初始化为0
	memset(map, 0, sizeof(map));

	//加载植物卡牌
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		//生成植物卡牌的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCard[i], name, 60, 80);//确定卡牌导入的宽高

		//导入植物动画逐帧卡片
		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			//先判断这个文件是否存在
			if (fileExist(name))
			{
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name);
			}
			else
			{
				break;
			}
		}
	}


	//初始化阳光池
	memset(balls, 0, sizeof(balls));

	for (int i = 0; i < 29; i++)
	{
		//导入阳光球逐帧动画
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//初始化僵尸池
	memset(zms, 0, sizeof(zms));

	for (int i = 0; i < 22; i++)
	{
		//导入僵尸逐帧动画
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM_Walk[i], name);
	}

	//初始化僵尸死亡逐帧动画
	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZM_Dead[i], name);
	}

	//初始化僵尸吃植物的逐帧动画
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZM_Eat[i], name);
	}

	//初始化子弹
	memset(bullets, 0, sizeof(bullets));

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");

	//初始化豌豆子弹的帧图片数组
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[3].getwidth() * k,
			imgBulletBlast[3].getheight() * k, true);
	}

	//初始化当前植物
	curZhiWu = 0;

	//创建游戏窗口
	initgraph(WIN_WIDTH, WIN_HEIGTH);

	//设置字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");//改变字体
	f.lfQuality = ANTIALIASED_QUALITY; //抗锯齿效果
	settextstyle(&f); //把字体设置进去
	setbkmode(TRANSPARENT); //设置背景透明
	setcolor(BLACK);
}


//更新游戏窗口
void updateWindow()
{
	BeginBatchDraw();//开始双缓冲，防止画面闪烁
	mciSendString("close res/bg.mp3", 0, 0, 0);

	putimage(0, 0, &imgBg);//显示背景
	putimagePNG(260 , 0, &imgBar);//显示工具栏

	//打印阳光值
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshineNum);
	outtextxy(287, 68, scoreText);  //输出分数

	//显示植物卡片
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		int x = 355 + i * 63;//355为放置第一张植物卡片的坐标，63表示下一张坐标与上一张坐标的坐标差
		int y = 12;
		putimagePNG(x, y, &imgCard[i]);

	}

	//打印地图上的植物
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type != 0)
			{
				putimagePNG(map[i][j].x, map[i][j].y, imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex]);

				//if (imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex + 1])//判断还有下一帧，则帧数加一
				//{
				//	map[i][j].frameIndex++;
				//}
				//else
				//{
				//	map[i][j].frameIndex = 0;
				//}
			}
		}
	}

	//渲染拖动过程中的植物
	if (curZhiWu > 0)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}


	//打印阳光球
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used)
		{
			int index = balls[i].frameIndex;
			putimagePNG(balls[i].x, balls[i].y, &imgSunshineBall[index]);
		}
		else if (balls[i].leap)
		{
			int index = balls[i].frameIndex;
			putimagePNG(balls[i].x, balls[i].y, &imgSunshineBall[index]);
		}
	}

	//打印僵尸
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmMax; i++)
	{
		if (zms[i].used == true)
		{
			if (zms[i].dead)//处于死亡状态
			{
				int index = zms[i].frameIndex;
				putimagePNG(zms[i].x, zms[i].y, &imgZM_Dead[index]);
			}
			else if (zms[i].eating)//处于吃饭状态
			{
				int index = zms[i].frameIndex;
				putimagePNG(zms[i].x, zms[i].y, &imgZM_Eat[index]);
			}
			else//处于行走状态
			{
				int index = zms[i].frameIndex;
				putimagePNG(zms[i].x, zms[i].y, &imgZM_Walk[index]);
			}
		}
	}

	//打印子弹
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletsMax; i++)
	{
		if (bullets[i].used)
		{
			if (bullets[i].balst)//爆炸状态
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletBlast[bullets[i].frameIndex]);
			}
			else//正常状态
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
	
	EndBatchDraw();//结束双缓冲
}

//收集阳光的函数
void collectSunshine(const ExMessage* msg)
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used)
		{
			int index = balls[i].frameIndex;
			if (msg->x > balls[i].x && msg->x<balls[i].x + imgSunshineBall[index].getwidth() && msg->y>balls[i].y && msg->y < balls[i].y + imgSunshineBall[index].getheight())
			{
				mciSendString("play res/sunshine.mp3", 0, 0, 0);

				balls[i].timer = 0;

				//设置阳光球偏移量
				double destX = 285;
				double destY = 20;
				double angle = atan((balls[i].y - destY) / (balls[i].x - destX));//计算出角度
				balls[i].xoff = 8 * cos(angle);
				balls[i].yoff = 8 * sin(angle);
				balls[i].leap = true;
			}
		}
	}
}

//记录用户鼠标信息
void userClick()
{
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg))//判断有没有消息,有消息就保存在msg里
	{
		if (msg.message == WM_LBUTTONDOWN)//如果是左键按下
		{
			if (msg.x > 355 && msg.x < 355 + ZHI_WU_COUNT * 63 && msg.y>12 && msg.y < 92)
			{
				curX = msg.x;
				curY = msg.y;
				curZhiWu = (msg.x - 355) / 63 + 1;
				status = 1;//左键按下表示状态为1
			}
			else
			{
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)//鼠标移动
		{
			//更新鼠标位置
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP)//左键抬起
		{
			if (msg.y > 180 && msg.y < 490 && msg.x>255 && msg.x < 990)//判断不超出地图
			{
				int row = (msg.y - 180) / 95;
				int col = (msg.x - 255) / 80;

				if (map[row][col].type == 0)//如果当前位置没有植物种植
				{
					//更新地图
					map[row][col].x = 255 + col * 80;
					map[row][col].y = 180 + row * 95;
					map[row][col].type = curZhiWu;
					map[row][col].frameIndex = 0;
					map[row][col].blood = 100;
					map[row][col].catched = false;
					map[row][col].timer = 0;
				}
			}
			
			//重新初始化植物和鼠标状态
			curZhiWu = 0;
			status = 0;
		}
	}
}


//更新植物动画帧数
void updateZhiWuFrame()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type != 0)
			{
				if (imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex + 1])//判断还有下一帧，则帧数加一
				{
					map[i][j].frameIndex++;
				}
				else
				{
					map[i][j].frameIndex = 0;
				}
			}
		}
	}
}


//更新阳光球坐标和动画帧
void updateSunshineBall()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used == true)
		{
			//更新阳光球动画帧
			if (balls[i].frameIndex < 28)
			{
				balls[i].frameIndex++;
			}
			else
			{
				balls[i].frameIndex = 0;
			}

			//更新阳光球位置
			if (balls[i].leap)//如果阳光球处于飞跃状态，坐标减去偏移量
			{
				if (balls[i].x < 285 && balls[i].y < 20)//如果阳光球已经到达指定位置，改变其状态为未在用状态
				{
					balls[i].leap = false;
					balls[i].used = false;
					sunshineNum += 25;
				}
				else
				{
					balls[i].x -= balls[i].xoff;

					balls[i].y -= balls[i].yoff;
					if (balls[i].y < 0)
					{
						balls[i].y = 0;
					}
				}
			}
			else if (balls[i].y <= balls[i].targetY)
			{
				balls[i].y += 2;
			}
			else
			{
				balls[i].timer += 1;

				if (balls[i].timer > 500)//500帧之后阳光球作废
				{
					balls[i].used = false;//该阳光球作废
					balls[i].timer = 0;
				}
			}
		}
	}
}

//更新僵尸坐标和动画帧
void updateZM()
{
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	for (int i = 0; i < zmMax; i++)
	{
		if (zms[i].used == true)
		{
			int lineZhiWu = -1;//记录在此僵尸前面的第一颗植物(碰撞状态 或者 未碰撞状态)
			for (int j = 8; j >= 0; j--)
			{
				if (map[zms[i].row][j].type != 0)
				{
					int ZhiWu_X = map[zms[i].row][j].x;
					if (zms[i].x + 85 >= ZhiWu_X)
					{
						lineZhiWu = j;
						break;
					}
				}
			}

			if (zms[i].dead)//僵尸已死亡，播放动画
			{
				zms[i].frameIndex++;
				if (zms[i].frameIndex > 19)
				{
					zms[i].used = false;
					zms[i].dead = false;
					zms[i].eating = false;
				}
			}
			else if (zms[i].eating)//僵尸处于使用状态，未死亡，且处于吃植物状态
			{
				int row = zms[i].row;
				map[row][lineZhiWu].bloodTimer++;

				if (map[row][lineZhiWu].bloodTimer > 10)//每过十次对植物做伤害一次
				{
					map[row][lineZhiWu].bloodTimer = 0;//计时器归零

					map[row][lineZhiWu].blood -= zms[i].harm;//对植物做伤害
					if (map[row][lineZhiWu].blood <= 0)//植物死亡
					{
						zms[i].eating = false;//僵尸恢复正常状态
						map[row][lineZhiWu].blood = 0;
						map[row][lineZhiWu].catched = false;
						map[row][lineZhiWu].frameIndex = 0;
						map[row][lineZhiWu].timer = 0;
						map[row][lineZhiWu].type = 0;
						map[row][lineZhiWu].x = 0;
						map[row][lineZhiWu].y = 0;
					}
				}
				

				//更新吃植物动画帧
				if (zms[i].frameIndex < 20)
				{
					zms[i].frameIndex++;
				}
				else
				{
					zms[i].frameIndex = 0;
				}
			}
			else//僵尸在使用，未死，未在吃，则正常更新僵尸行走帧数和坐标
			{
				//更新僵尸动画帧
				if (zms[i].frameIndex < 21)
				{
					zms[i].frameIndex++;
				}
				else
				{
					zms[i].frameIndex = 0;
				}

				//更新僵尸位置
				if (zms[i].x >= 180)
				{
					zms[i].x -= zms[i].speed;
					int row = zms[i].row;
					int linesZhiWu_X = (lineZhiWu >= 0) ? (map[row][lineZhiWu].x + 68) : 0;
					if (zms[i].x + 85 < linesZhiWu_X)//僵尸与植物接触
					{
						zms[i].eating = true;
						zms[i].frameIndex = 0;

						map[row][lineZhiWu].catched = true;
					}
				}
				else
				{
					printf("gameover\n");
					zms[i].used = false;
					zms[i].frameIndex = 0;
					//待完善
				}
			}
		}
	}

}

//设置启动菜单
void startUI()
{
	//加载菜单图片
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");


	int flag = 0;//记录是否选中，选中为1，未选中为0
	while (1)
	{
		BeginBatchDraw();
		mciSendString("open res/bg.mp3", 0, 0, 0);
		mciSendString("play res/bg.mp3", 0, 0, 0);//播放背景音乐，在进入updateWindow函数后关闭


		putimage(0, 0, &imgBg);
		putimagePNG(470, 145, (flag == 1) ? &imgMenu1 : &imgMenu2);
	

		ExMessage msg;//记录鼠标信息
		if (peekmessage(&msg))//如果有信息，记录在msg里
		{
			if (msg.x > 470 && msg.x < 470 + imgMenu1.getwidth() && msg.y>145 && msg.y < 145 + imgMenu1.getheight())//如果鼠标在操作区域
			{
				if ((msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN))//如果左键进入(移动或左键点下)
				{
					flag = 1;
				}
				else if (msg.message == WM_LBUTTONUP)//左键在操作区域松开，则进入游戏
				{
					return;
				}
			}
			else
			{
				flag = 0;
			}
		}


		EndBatchDraw();
	}
}


//创建阳光球
void creatSunshineBall()
{
	//阳光池中有多少个
	int ballMax = sizeof(balls) / sizeof(balls[0]);

	//从阳光池中取一个能用的
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used == false)
		{
			balls[i].used = true;
			balls[i].frameIndex = 0;
			balls[i].x = rand() % (950 - 285) + 285;  //确定初始x坐标
			balls[i].y = rand() % 50;  //确定初始y坐标坐标在上面
			balls[i].targetY = rand() % (490 - 180) + 180;  //确定目标y坐标
			balls[i].leap = false;
			balls[i].xoff = 0;
			balls[i].yoff = 0;

			break;//每次只调用一个阳光球
		}
	}

}

//创建僵尸
void creatZM()
{
	//总共有几个僵尸
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	if (all_zm_num == 0)
	{
		int check = 1;
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)
			{
				check = 0;
				break;
			}
		}
		if (check)
		{
			putimage(0, 0, &game_win);
			Sleep(10000);
			exit(0);
		}
		return;
	}
	else
	{
		all_zm_num--;
	}

	//找一个可以登场的僵尸登场
	for (int i = 0; i < zmMax; i++)
	{
		if (zms[i].used == false)
		{
			zms[i].used = true;
			zms[i].x = WIN_WIDTH - imgZM_Walk[0].getwidth();
			zms[i].row = rand() % 3;
			zms[i].y = 105 + zms[i].row * 102;
			zms[i].speed = 1;
			zms[i].blood = 100;
			zms[i].dead = false;
			zms[i].frameIndex = 0;
			zms[i].eating = false;
			zms[i].eating = false;
			zms[i].harm = 25;
			break;
		}
	}
}


//发射子弹
void shoot()
{
	int lines[3] = { 0 }; //记录每一行的僵尸个数
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = 890;  //危险距离
	for (int i = 0; i < zmMax; i++)
	{
		if (zms[i].used && zms[i].x < dangerX)//僵尸存在并且在射击区域内
		{
			int row = zms[i].row;
			lines[row]++;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type - 1 == WAN_DOU && lines[i] > 0)//为豌豆并有僵尸
			{
				map[i][j].timer++;
				if (map[i][j].timer > 500)//每20帧发射一次子弹
				{
					map[i][j].timer = 0;

					for (int k = 0; k < bulletsMax; k++)
					{
						if (bullets[k].used == false)
						{
							bullets[k].used = true;
							bullets[k].row = i;
							bullets[k].speed = 4;
							bullets[k].timer = 0;
							bullets[k].x = map[i][j].x + imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex]->getwidth() - 12;
							bullets[k].y = map[i][j].y + 8;
							bullets[k].harm = 25;
							bullets[k].frameIndex = 0;
							bullets[k].balst = false;
							break;
						}
					}
				}
			}
		}
	}
}

//更新子弹位置数据
void updateBullet()
{
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	int zmsMax = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bulletsMax; i++)
	{
		if (bullets[i].used)
		{
			//确定距离子弹最近的僵尸
			int nearestX = WIN_WIDTH;
			int nearestZM = -1;
			for (int j = 0; j < zmsMax; j++)
			{
				if (zms[j].used && zms[j].dead == false && zms[j].row == bullets[i].row && zms[j].x + 100 >= bullets[i].x)//确定子弹所在行僵尸的最近距离，并确保子弹位于僵尸的左边
				{
					if (zms[j].x + 100 < nearestX)
					{
						nearestX = zms[j].x + 100;
						nearestZM = j;
					}
				}
			}


			if (bullets[i].balst)//如果子弹是处于爆炸状态，则需处理的是爆炸动画的帧数
			{
				bullets[i].timer++;
				if (bullets[i].timer > 20)
				{
					bullets[i].timer = 0;
					bullets[i].frameIndex++;
					if (bullets[i].frameIndex > 3)
					{
						bullets[i].used = false;
						bullets[i].balst = false;
						bullets[i].frameIndex = 0;
					}
				}

			}
			else//子弹处于使用状态但不在爆炸状态，则需处理的是子弹的坐标信息
			{
				if ((bullets[i].x + imgBulletNormal.getwidth()) >= nearestX && (bullets[i].x + imgBulletNormal.getwidth()) < nearestX + 25)//僵尸和子弹发生碰撞
				{
					bullets[i].balst = true;
					bullets[i].timer = 0;
					zms[nearestZM].blood -= bullets[i].harm;
					if (zms[nearestZM].blood <= 0)
					{
						zms[nearestZM].dead = true;
						zms[i].eating = false;
						zms[nearestZM].frameIndex = 0;
					}
				}
				else
				{
					bullets[i].timer++;
					if (bullets[i].timer > 15)//每过15帧，更新子弹位置
					{
						bullets[i].timer = 0;
						bullets[i].x += bullets[i].speed;
						if (bullets[i].x > 990)
						{
							bullets[i].used = false;
						}
					}
				}
			}
		}
	}
}



int main(void)
{
	srand(time(NULL));	//配置随机种子

	gameInit(); //游戏初始化

	startUI(); //显示菜单

	int timer0 = 0;//定义一个计时器，确定更新植物帧数的时间间隔
	int timer1 = 0;//定义一个计时器，确定创造阳光球的时间间隔
	int timer2 = 0; //定义一个计时器，确定僵尸位置更新的时间
	while (1)
	{
		userClick();
		int timeChange = getDelay();  //getDelay()记录前后两次调用该函数的时间间隔
		timer0 += timeChange;
		timer1 += timeChange;
		timer2 += timeChange;

		if (timer0 > 40)//累积过了40ms，更新一次植物帧数
		{
			timer0 = 0;//计时器清零，重新记录间隔时间

			updateZhiWuFrame();
			updateSunshineBall();
		}

		if (timer1 > 5000)
		{
			timer1 = 0;
			creatSunshineBall();
			creatZM();
		}

		if (timer2 > 40)
		{
			timer2 = 0;
			updateZM();
		}


		shoot();
		updateBullet();

		updateWindow();//更新游戏窗口
	}


	return 0;
}