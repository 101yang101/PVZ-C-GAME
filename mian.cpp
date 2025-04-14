#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <time.h>
#include <math.h>
#include "tools.h"
#include "vector2.h"

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib");//����⣬���Բ�����Ч


#define WIN_WIDTH 900
#define WIN_HEIGTH 600

int all_zm_num = 20;

enum {WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT};

IMAGE imgBg;//��ʾ����ͼƬ
IMAGE imgBar;//������
IMAGE imgCard[ZHI_WU_COUNT];  //ֲ�￨��
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];  //����ֲ����֡��Ƭ
IMAGE imgSunshineBall[29];  //���������֡����
IMAGE imgZM_Walk[22];  //��ʬ���ߵ���֡����
IMAGE imgZM_Dead[20];  //��ʬ��������֡����
IMAGE imgZM_Eat[21];  //��ʬ��ֲ�����֡����
IMAGE imgBulletNormal; //����״̬���ӵ��Ŀ�Ƭ
IMAGE imgBulletBlast[4];//�ӵ���ը
IMAGE game_win;

int sunshineNum; //����ֵ
int curZhiWu; //0��ʾû��ѡ�У�1��ʾѡ�е�һ��ֲ��
int curX, curY;//��ǰ�ƶ�������ֲ�������

struct zhiwu
{
	int x, y;
	int type;//��ʾ�ڼ���ֲ��
	int frameIndex; //����֡�����
	
	bool catched;//�Ƿ񱻽�ʬ����
	int blood; //ֲ��Ѫ��
	int bloodTimer; //Ѫ����ʱ������һ���������Ѫ��

	int sunshineTimer; //��¼���տ������������ʱ����

	//����Ϊ������ֲ�������
	int timer;//��ʱ������¼�����ϴη����ӵ���ʱ����
};

struct zhiwu map[3][9];//��ͼ��ֲ��ֲ�


//������ÿ��������25������ֵ
struct sunshineBall
{
	int x; //��ǰ����
	int y;

	int targetY;//Ŀ��yλ�ã���ֱ����
	
	int frameIndex; //����֡����

	bool used;//�жϸ�����������������Ƿ�ʹ��

	int timer;//��ʱ�����ӵ������ݵؿ�ʼ��ʱ������500֮֡��û������������

	bool leap;//�Ƿ��ڷ�Ծ״̬
	double xoff; //��Ծ������x��y�����ƫ����
	double yoff;
};


//�����
struct sunshineBall balls[10];

//��ʬ
struct zm
{
	int x, y;//y = row*100 + 180
	int row;//��ʬ�ߵڼ���
	int frameIndex;
	bool used;
	int speed;
	int blood;//��ʬѪ��
	int harm; //��ʬ��ֲ����˺�ֵ
	bool dead;//�ж��Ƿ�����
	bool eating; //�ж��Ƿ����ڳ�ֲ��
};

//��ʬ��
struct zm zms[10];

//�ӵ�
struct bullet
{
	int x, y;
	int row;
	bool used;
	int speed;
	int harm;

	bool balst;//�Ƿ��ڱ�ը״̬
	int frameIndex;//��ը����֡��
	int timer;//��ʱ��
};

//�ӵ���
struct bullet bullets[30];

//�ж��ļ��Ƿ����
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


//��Ϸ��ʼ��
void gameInit()
{
	//���ַ����޸�Ϊ�����ֽ��ַ�����

	//��ʼ������ֵ
	sunshineNum = 100;

	//������Ϸ����ͼƬ
	loadimage(&imgBg, "res/bg.jpg");

	//���ع�����ͼƬ
	loadimage(&imgBar, "res/bar5.png");

	//����ֲ�ﶯ��,������forѭ����
	memset(imgZhiWu, 0, sizeof(imgZhiWu));

	//����ʤ������
	loadimage(&game_win, "res/gameWin.png");

	//��ͼ��ֲ���ʼ��Ϊ0
	memset(map, 0, sizeof(map));

	//����ֲ�￨��
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		//����ֲ�￨�Ƶ��ļ���
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCard[i], name, 60, 80);//ȷ�����Ƶ���Ŀ��

		//����ֲ�ﶯ����֡��Ƭ
		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			//���ж�����ļ��Ƿ����
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


	//��ʼ�������
	memset(balls, 0, sizeof(balls));

	for (int i = 0; i < 29; i++)
	{
		//������������֡����
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//��ʼ����ʬ��
	memset(zms, 0, sizeof(zms));

	for (int i = 0; i < 22; i++)
	{
		//���뽩ʬ��֡����
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM_Walk[i], name);
	}

	//��ʼ����ʬ������֡����
	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZM_Dead[i], name);
	}

	//��ʼ����ʬ��ֲ�����֡����
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZM_Eat[i], name);
	}

	//��ʼ���ӵ�
	memset(bullets, 0, sizeof(bullets));

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");

	//��ʼ���㶹�ӵ���֡ͼƬ����
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[3].getwidth() * k,
			imgBulletBlast[3].getheight() * k, true);
	}

	//��ʼ����ǰֲ��
	curZhiWu = 0;

	//������Ϸ����
	initgraph(WIN_WIDTH, WIN_HEIGTH);

	//��������
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");//�ı�����
	f.lfQuality = ANTIALIASED_QUALITY; //�����Ч��
	settextstyle(&f); //���������ý�ȥ
	setbkmode(TRANSPARENT); //���ñ���͸��
	setcolor(BLACK);
}


//������Ϸ����
void updateWindow()
{
	BeginBatchDraw();//��ʼ˫���壬��ֹ������˸
	mciSendString("close res/bg.mp3", 0, 0, 0);

	putimage(0, 0, &imgBg);//��ʾ����
	putimagePNG(260 , 0, &imgBar);//��ʾ������

	//��ӡ����ֵ
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshineNum);
	outtextxy(287, 68, scoreText);  //�������

	//��ʾֲ�￨Ƭ
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		int x = 355 + i * 63;//355Ϊ���õ�һ��ֲ�￨Ƭ�����꣬63��ʾ��һ����������һ������������
		int y = 12;
		putimagePNG(x, y, &imgCard[i]);

	}

	//��ӡ��ͼ�ϵ�ֲ��
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type != 0)
			{
				putimagePNG(map[i][j].x, map[i][j].y, imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex]);

				//if (imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex + 1])//�жϻ�����һ֡����֡����һ
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

	//��Ⱦ�϶������е�ֲ��
	if (curZhiWu > 0)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}


	//��ӡ������
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

	//��ӡ��ʬ
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmMax; i++)
	{
		if (zms[i].used == true)
		{
			if (zms[i].dead)//��������״̬
			{
				int index = zms[i].frameIndex;
				putimagePNG(zms[i].x, zms[i].y, &imgZM_Dead[index]);
			}
			else if (zms[i].eating)//���ڳԷ�״̬
			{
				int index = zms[i].frameIndex;
				putimagePNG(zms[i].x, zms[i].y, &imgZM_Eat[index]);
			}
			else//��������״̬
			{
				int index = zms[i].frameIndex;
				putimagePNG(zms[i].x, zms[i].y, &imgZM_Walk[index]);
			}
		}
	}

	//��ӡ�ӵ�
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletsMax; i++)
	{
		if (bullets[i].used)
		{
			if (bullets[i].balst)//��ը״̬
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletBlast[bullets[i].frameIndex]);
			}
			else//����״̬
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
	
	EndBatchDraw();//����˫����
}

//�ռ�����ĺ���
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

				//����������ƫ����
				double destX = 285;
				double destY = 20;
				double angle = atan((balls[i].y - destY) / (balls[i].x - destX));//������Ƕ�
				balls[i].xoff = 8 * cos(angle);
				balls[i].yoff = 8 * sin(angle);
				balls[i].leap = true;
			}
		}
	}
}

//��¼�û������Ϣ
void userClick()
{
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg))//�ж���û����Ϣ,����Ϣ�ͱ�����msg��
	{
		if (msg.message == WM_LBUTTONDOWN)//������������
		{
			if (msg.x > 355 && msg.x < 355 + ZHI_WU_COUNT * 63 && msg.y>12 && msg.y < 92)
			{
				curX = msg.x;
				curY = msg.y;
				curZhiWu = (msg.x - 355) / 63 + 1;
				status = 1;//������±�ʾ״̬Ϊ1
			}
			else
			{
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)//����ƶ�
		{
			//�������λ��
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP)//���̧��
		{
			if (msg.y > 180 && msg.y < 490 && msg.x>255 && msg.x < 990)//�жϲ�������ͼ
			{
				int row = (msg.y - 180) / 95;
				int col = (msg.x - 255) / 80;

				if (map[row][col].type == 0)//�����ǰλ��û��ֲ����ֲ
				{
					//���µ�ͼ
					map[row][col].x = 255 + col * 80;
					map[row][col].y = 180 + row * 95;
					map[row][col].type = curZhiWu;
					map[row][col].frameIndex = 0;
					map[row][col].blood = 100;
					map[row][col].catched = false;
					map[row][col].timer = 0;
				}
			}
			
			//���³�ʼ��ֲ������״̬
			curZhiWu = 0;
			status = 0;
		}
	}
}


//����ֲ�ﶯ��֡��
void updateZhiWuFrame()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type != 0)
			{
				if (imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex + 1])//�жϻ�����һ֡����֡����һ
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


//��������������Ͷ���֡
void updateSunshineBall()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used == true)
		{
			//���������򶯻�֡
			if (balls[i].frameIndex < 28)
			{
				balls[i].frameIndex++;
			}
			else
			{
				balls[i].frameIndex = 0;
			}

			//����������λ��
			if (balls[i].leap)//����������ڷ�Ծ״̬�������ȥƫ����
			{
				if (balls[i].x < 285 && balls[i].y < 20)//����������Ѿ�����ָ��λ�ã��ı���״̬Ϊδ����״̬
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

				if (balls[i].timer > 500)//500֮֡������������
				{
					balls[i].used = false;//������������
					balls[i].timer = 0;
				}
			}
		}
	}
}

//���½�ʬ����Ͷ���֡
void updateZM()
{
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	for (int i = 0; i < zmMax; i++)
	{
		if (zms[i].used == true)
		{
			int lineZhiWu = -1;//��¼�ڴ˽�ʬǰ��ĵ�һ��ֲ��(��ײ״̬ ���� δ��ײ״̬)
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

			if (zms[i].dead)//��ʬ�����������Ŷ���
			{
				zms[i].frameIndex++;
				if (zms[i].frameIndex > 19)
				{
					zms[i].used = false;
					zms[i].dead = false;
					zms[i].eating = false;
				}
			}
			else if (zms[i].eating)//��ʬ����ʹ��״̬��δ�������Ҵ��ڳ�ֲ��״̬
			{
				int row = zms[i].row;
				map[row][lineZhiWu].bloodTimer++;

				if (map[row][lineZhiWu].bloodTimer > 10)//ÿ��ʮ�ζ�ֲ�����˺�һ��
				{
					map[row][lineZhiWu].bloodTimer = 0;//��ʱ������

					map[row][lineZhiWu].blood -= zms[i].harm;//��ֲ�����˺�
					if (map[row][lineZhiWu].blood <= 0)//ֲ������
					{
						zms[i].eating = false;//��ʬ�ָ�����״̬
						map[row][lineZhiWu].blood = 0;
						map[row][lineZhiWu].catched = false;
						map[row][lineZhiWu].frameIndex = 0;
						map[row][lineZhiWu].timer = 0;
						map[row][lineZhiWu].type = 0;
						map[row][lineZhiWu].x = 0;
						map[row][lineZhiWu].y = 0;
					}
				}
				

				//���³�ֲ�ﶯ��֡
				if (zms[i].frameIndex < 20)
				{
					zms[i].frameIndex++;
				}
				else
				{
					zms[i].frameIndex = 0;
				}
			}
			else//��ʬ��ʹ�ã�δ����δ�ڳԣ����������½�ʬ����֡��������
			{
				//���½�ʬ����֡
				if (zms[i].frameIndex < 21)
				{
					zms[i].frameIndex++;
				}
				else
				{
					zms[i].frameIndex = 0;
				}

				//���½�ʬλ��
				if (zms[i].x >= 180)
				{
					zms[i].x -= zms[i].speed;
					int row = zms[i].row;
					int linesZhiWu_X = (lineZhiWu >= 0) ? (map[row][lineZhiWu].x + 68) : 0;
					if (zms[i].x + 85 < linesZhiWu_X)//��ʬ��ֲ��Ӵ�
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
					//������
				}
			}
		}
	}

}

//���������˵�
void startUI()
{
	//���ز˵�ͼƬ
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");


	int flag = 0;//��¼�Ƿ�ѡ�У�ѡ��Ϊ1��δѡ��Ϊ0
	while (1)
	{
		BeginBatchDraw();
		mciSendString("open res/bg.mp3", 0, 0, 0);
		mciSendString("play res/bg.mp3", 0, 0, 0);//���ű������֣��ڽ���updateWindow������ر�


		putimage(0, 0, &imgBg);
		putimagePNG(470, 145, (flag == 1) ? &imgMenu1 : &imgMenu2);
	

		ExMessage msg;//��¼�����Ϣ
		if (peekmessage(&msg))//�������Ϣ����¼��msg��
		{
			if (msg.x > 470 && msg.x < 470 + imgMenu1.getwidth() && msg.y>145 && msg.y < 145 + imgMenu1.getheight())//�������ڲ�������
			{
				if ((msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN))//����������(�ƶ����������)
				{
					flag = 1;
				}
				else if (msg.message == WM_LBUTTONUP)//����ڲ��������ɿ����������Ϸ
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


//����������
void creatSunshineBall()
{
	//��������ж��ٸ�
	int ballMax = sizeof(balls) / sizeof(balls[0]);

	//���������ȡһ�����õ�
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used == false)
		{
			balls[i].used = true;
			balls[i].frameIndex = 0;
			balls[i].x = rand() % (950 - 285) + 285;  //ȷ����ʼx����
			balls[i].y = rand() % 50;  //ȷ����ʼy��������������
			balls[i].targetY = rand() % (490 - 180) + 180;  //ȷ��Ŀ��y����
			balls[i].leap = false;
			balls[i].xoff = 0;
			balls[i].yoff = 0;

			break;//ÿ��ֻ����һ��������
		}
	}

}

//������ʬ
void creatZM()
{
	//�ܹ��м�����ʬ
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

	//��һ�����Եǳ��Ľ�ʬ�ǳ�
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


//�����ӵ�
void shoot()
{
	int lines[3] = { 0 }; //��¼ÿһ�еĽ�ʬ����
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = 890;  //Σ�վ���
	for (int i = 0; i < zmMax; i++)
	{
		if (zms[i].used && zms[i].x < dangerX)//��ʬ���ڲ��������������
		{
			int row = zms[i].row;
			lines[row]++;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type - 1 == WAN_DOU && lines[i] > 0)//Ϊ�㶹���н�ʬ
			{
				map[i][j].timer++;
				if (map[i][j].timer > 500)//ÿ20֡����һ���ӵ�
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

//�����ӵ�λ������
void updateBullet()
{
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	int zmsMax = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bulletsMax; i++)
	{
		if (bullets[i].used)
		{
			//ȷ�������ӵ�����Ľ�ʬ
			int nearestX = WIN_WIDTH;
			int nearestZM = -1;
			for (int j = 0; j < zmsMax; j++)
			{
				if (zms[j].used && zms[j].dead == false && zms[j].row == bullets[i].row && zms[j].x + 100 >= bullets[i].x)//ȷ���ӵ������н�ʬ��������룬��ȷ���ӵ�λ�ڽ�ʬ�����
				{
					if (zms[j].x + 100 < nearestX)
					{
						nearestX = zms[j].x + 100;
						nearestZM = j;
					}
				}
			}


			if (bullets[i].balst)//����ӵ��Ǵ��ڱ�ը״̬�����账����Ǳ�ը������֡��
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
			else//�ӵ�����ʹ��״̬�����ڱ�ը״̬�����账������ӵ���������Ϣ
			{
				if ((bullets[i].x + imgBulletNormal.getwidth()) >= nearestX && (bullets[i].x + imgBulletNormal.getwidth()) < nearestX + 25)//��ʬ���ӵ�������ײ
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
					if (bullets[i].timer > 15)//ÿ��15֡�������ӵ�λ��
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
	srand(time(NULL));	//�����������

	gameInit(); //��Ϸ��ʼ��

	startUI(); //��ʾ�˵�

	int timer0 = 0;//����һ����ʱ����ȷ������ֲ��֡����ʱ����
	int timer1 = 0;//����һ����ʱ����ȷ�������������ʱ����
	int timer2 = 0; //����һ����ʱ����ȷ����ʬλ�ø��µ�ʱ��
	while (1)
	{
		userClick();
		int timeChange = getDelay();  //getDelay()��¼ǰ�����ε��øú�����ʱ����
		timer0 += timeChange;
		timer1 += timeChange;
		timer2 += timeChange;

		if (timer0 > 40)//�ۻ�����40ms������һ��ֲ��֡��
		{
			timer0 = 0;//��ʱ�����㣬���¼�¼���ʱ��

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

		updateWindow();//������Ϸ����
	}


	return 0;
}