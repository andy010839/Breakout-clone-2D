
#include <windows.h>
#include <gl/glut.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <time.h>
#include "object.h"

using namespace std;

const float PI = 3.14159;
int ww = 600, wh = 800;

//磚塊  
vector<Block> blocks;
//球
vector<Ball> balls;
//板子
Bar bar;
//道具
vector<Bonus> bonuses;
enum BONUS_TYPE
{
    BALL_SPEED_UP,		//0
    BALL_SPEED_DOWN,	//1
	BALL_SPEED_RECOVER,	//2
    BALL_SIZE_UP,		//3
	BALL_SIZE_DOWN,		//4
    BALL_SIZE_RECOVER,	//5
	BAR_SIZE_UP,		//6
	BAR_SIZE_DOWN,		//7
	BALL_NUMBER_UP,		//8
	BALL_NUMBER_INFINITE//9
};
bool ball_infinite;		//球無限控制

//鍵盤控制
bool keyLeft;
bool keyRight;

//函式宣告
//移動方向與碰撞反彈
void ball_movement(void);
//碰撞偵測
void Collision_Ball_and_Block(void);
void Collision_Ball_and_Bar(void);
void Collision_Bar_and_Border(void);
void Collision_Bar_and_Bonus(void);
//顯示
void display(void);
void display_GameOver(void);
void display_Clear(void);
//鍵盤事件
void keyboard(unsigned char key, int x, int y);
void keyboardS(int key, int x, int y);
void keyboardSUp(int key, int x, int y);
//常駐事件
void idle(void);

//讀檔
void loadMap()
{
	//讀取檔案
	FILE *fp;
	char filename[30] = "map.txt";
	fp= fopen(filename, "r");
	
	char c;	//作為存放map檔的字元
	int block_location_x = 30;	//初始磚塊的中心點ｘ座標
	int block_location_y = 790;	//初始磚塊的中心點ｙ座標
	
	if(!blocks.empty())								//如果磚塊不是空的
		blocks.erase(blocks.begin(),blocks.end());	//刪掉所有磚塊才重畫

	while( (c = fgetc(fp))!=EOF ) //一次讀取一個字元
	{
		//設定每一個磚塊的資訊
		if(c >= '1' && c <= '9')
		{
			Block b;
			b.setLocation(block_location_x,block_location_y);
			b.setSize(60,20);
			b.hp = c - '0';
			b.setColorFromHP();

			blocks.push_back(b);
		}
		block_location_x += 60;	//無論是否有畫磚塊 均把ｘ座標移至下一中心點	

		if(c == '\n')	//每讀了10次字元就要切換到下一行
		{
			block_location_x = 30;	//重設ｘ座標
			block_location_y -= 20; //到下一排磚塊中心點ｙ座標
		}
	}
	fclose(fp);
}

//初始設定
void myinit()
{ 
	glClearColor(1.0, 1.0, 1.0, 1.0); //背景顏色

	//場景大小
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 600.0, 0.0, 800.0);

	//攝影機視角
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//設定磚塊的資訊
	loadMap();

	//設定球的資訊
	if(!balls.empty())									//如果球不是零個
		balls.erase(balls.begin(),balls.end());			//先設成零個
	Ball _ball;
	_ball.speed = 5;	//球的初速
	_ball.dx = cos(60 * PI/180) * _ball.speed;	//以三角函數方式改變dx dy變量 可確保速度值一樣
	_ball.dy = sin(60 * PI/180) * _ball.speed;
	_ball.setLocation(300,100);
	_ball.setSize(15,15);			
	_ball.setColor(0.9,0.0,0.0);
	balls.push_back(_ball);
	ball_infinite = false;

	//設定板子的資訊
	bar.setLocation(300,80);
	bar.setSize(150,10);
	bar.setColor(0.0,0.0,0.0);
	
	if(!bonuses.empty())								//如果道具不是空的
		bonuses.erase(bonuses.begin(),bonuses.end());	//刪掉所有道具

	//鍵盤設定
	keyLeft = false;
	keyRight = false;
}


//球的移動方向與碰撞反彈
void ball_movement(void)
{
	for(vector<Ball>::iterator i_ball = balls.begin() ; i_ball != balls.end() ; ) {
		//與邊界的碰撞偵測
		if( (i_ball -> x) + (i_ball -> width/2) >= ww) {	//右邊界判斷 
			i_ball -> dx = -cos(i_ball -> get_theta()) * (i_ball -> speed);	//將dx往左
//			ball.dx = -cos(45.0 * PI/180) * ball.speed;	//固定角度測試
		}
		if( (i_ball -> x) - (i_ball -> width/2) <= 0 ) {	//左邊界判斷
			i_ball -> dx = cos(i_ball -> get_theta()) * i_ball -> speed;	//將dx往右
//			ball.dx = cos(45.0 * PI/180) * ball.speed;	//固定角度測試
		}

		if( (i_ball -> y) + (i_ball -> height/2) >= wh) {	//上邊界判斷
			i_ball -> dy = -sin(i_ball -> get_theta()) * i_ball -> speed;	//將dy往下
//			ball.dy = -sin(45.0 * PI/180) * ball.speed;	//固定角度測試
		}

//		if( (i_ball -> y) + (i_ball -> height/2) <= 0 )		//下邊界判斷 碰撞彈回
//			ball.dy = sin(ball.get_theta()) * ball.speed;	//將dy往上
		if(i_ball -> y < 0) {						//下邊界判斷 出界後初始化 memo:要改球數量要把判斷改成如果場上沒有球
			i_ball = balls.erase(i_ball);
		}
		else {
			i_ball++;
		}
	}
	Collision_Ball_and_Block();	//與磚塊的碰撞偵測
	Collision_Ball_and_Bar();	//與板子的碰撞偵測

	for(unsigned int i = 0; i < balls.size(); i++) {
		balls[i].move();	//移動球
	}
}

//球與磚塊的碰撞偵測 
void Collision_Ball_and_Block(void)
{
	for(vector<Ball>::iterator i_ball = balls.begin() ; i_ball != balls.end() ; i_ball++) {		//對每個球		作判定
		for(vector<Block>::iterator i_block = blocks.begin() ; i_block != blocks.end() ; ) {	//對每個磚塊	作判定
			int x_of_ball_and_block = abs( i_ball -> x - (i_block -> x) );
			int y_of_ball_and_block = abs( i_ball -> y - (i_block -> y) );

			if( (x_of_ball_and_block <= i_ball -> width/2 + (i_block -> width)/2) && (y_of_ball_and_block <= i_ball -> height/2 + (i_block -> height)/2) ) {

				if(i_ball -> x > (i_block -> x) && y_of_ball_and_block <= (i_block -> height)/2)	//磚塊右側碰撞
					i_ball -> dx = cos(i_ball -> get_theta()) * i_ball -> speed;
				if(i_ball -> x < (i_block -> x) && y_of_ball_and_block <= (i_block -> height)/2)	//磚塊左側碰撞
					i_ball -> dx = -cos(i_ball -> get_theta()) * i_ball -> speed;
				if(i_ball -> y > (i_block -> y) && x_of_ball_and_block <= (i_block -> width)/2)		//磚塊上側碰撞
					i_ball -> dy = sin(i_ball -> get_theta()) * i_ball -> speed;
				if(i_ball -> y < (i_block -> y) && x_of_ball_and_block <= (i_block -> width)/2)		//磚塊下側碰撞
					i_ball -> dy = -sin(i_ball -> get_theta()) * i_ball -> speed;

				i_block -> hp -= 1;
				i_block -> setColorFromHP();
			
				if((i_block -> hp) <= 0) {
					//磚塊消失後產生道具
					int bonus_num = rand() % 20 + 1;	//1 ~ 20	可改變這邊讓道具掉落率改變
					if(bonus_num <= 16)	{	//隨機數 <=16 , 建立道具  
						Bonus bb;	
						bb.setLocation(i_block -> x,i_block -> y);
						bb.setSize(20,10);
						bb.dx = 0;
						bb.dy = -3;
	
						switch(bonus_num) {
							case 1:	//球加速
							case 2:
								bb.setBonus_Type(BALL_SPEED_UP);
								bb.setColor(1.0,0.2,0.8);	//粉紅
								break;
	
							case 3:	//球減速
							case 4:
								bb.setBonus_Type(BALL_SPEED_DOWN);
								bb.setColor(1.0,0.0,0.0);	//紅
								break;
		
							case 5:	//球初速
								bb.setBonus_Type(BALL_SPEED_RECOVER);
								bb.setColor(0.0,1.0,0.0);	//淺綠
								break;
	
							case 6:	//球變大
							case 7:
								bb.setBonus_Type(BALL_SIZE_UP);
								bb.setColor(0.0,0.2,1.0);	//深藍
								break;
	
							case 8:	//球變小
							case 9:
								bb.setBonus_Type(BALL_SIZE_DOWN);
								bb.setColor(0.2,1.0,1.0);	//淺藍
								break;

							case 10: //球初始大小
								bb.setBonus_Type(BALL_SIZE_RECOVER);
								bb.setColor(0.0,0.2,0.0);	//深綠
								break;
	
							case 11: //板子加長
							case 12:
								bb.setBonus_Type(BAR_SIZE_UP);
								bb.setColor(0.0,0.2,1.0);	//深藍
								break;
					
							case 13: //板子變短
							case 14:
								bb.setBonus_Type(BAR_SIZE_DOWN);
								bb.setColor(0.2,1.0,1.0);	//淺藍
								break;

							case 15: //球的數量增加 
								bb.setBonus_Type(BALL_NUMBER_UP);
								bb.setColor(0.0,0.0,0.0);	//黑
								break;
							case 16: //球數量無限
								bb.setBonus_Type(BALL_NUMBER_INFINITE);
								bb.setColor(1.0,1.0,0.0);	//黃
						}
						bonuses.push_back(bb);
					}	
					i_block = blocks.erase(i_block);	//消除hp = 0的磚塊
				}
				else {
					i_block++;
				}
			}
			else {
				i_block++;
			}
		}
	}
}


//球與板子的碰撞偵測 
void Collision_Ball_and_Bar(void)
{
	for(unsigned int i_ball = 0; i_ball < balls.size(); i_ball++) {	//對每個球 作判定
		int x_of_ball_and_bar = abs( balls[i_ball].x - bar.x );//取絕對值恆正
		int y_of_ball_and_bar = abs( balls[i_ball].y - bar.y );
	
		float part_x = 0.0;	//判斷球碰到板子的哪一區	
		float degree_of_part_x = 0.0;
			
		if( (y_of_ball_and_bar <= balls[i_ball].height/2 + bar.height/2) && (x_of_ball_and_bar <= balls[i_ball].width/2 + bar.width/2) ) {
//			ball.dy = sin(ball.get_theta()) * ball.speed;	//單純反彈入射角度

			part_x = (bar.x - balls[i_ball].x) + (balls[i_ball].width/2 + bar.width/2);	//使區域落在 (ball.width/2 + bar.width/2) ~ 0 之間
			degree_of_part_x = part_x *180/(balls[i_ball].width + bar.width);	//使區域落在 180 ~ 0 之間
			if(degree_of_part_x >= 160)	//使區域落在 160 ~ 20 之間 避免角度太平遊戲性差
				degree_of_part_x = 160;
			if(degree_of_part_x <= 20)
				degree_of_part_x = 20;
			printf("\n part_x：%f  degree_of_part_x：%f ",part_x,degree_of_part_x);	//測試用 用來查看接觸時的反彈度數
		
			balls[i_ball].dx = cos(degree_of_part_x * PI/180) * balls[i_ball].speed;
			balls[i_ball].dy = sin(degree_of_part_x * PI/180) * balls[i_ball].speed;
		}
	}
}

//板子與邊界的碰撞偵測
void Collision_Bar_and_Border(void)
{
	if(bar.x + bar.width/2 >= ww) 
		keyRight = false;
	if(bar.x - bar.width/2 <= 0 )
		keyLeft = false;
}
//板子與道具的碰撞偵測
void Collision_Bar_and_Bonus(void)
{
	for(vector<Bonus>::iterator i_bonus = bonuses.begin() ; i_bonus != bonuses.end(); ) {
		int x_of_bar_and_bonus = abs( bar.x - (i_bonus -> x) );
		int y_of_bar_and_bonus = abs( bar.y - (i_bonus -> y) );

		if(i_bonus -> y < 0) {	//出界控制
			i_bonus = bonuses.erase(i_bonus);
		}
		else if( (x_of_bar_and_bonus <= bar.width/2 + (i_bonus -> width)/2) && (y_of_bar_and_bonus <= bar.height/2 + (i_bonus -> height)/2) ) {
			for(unsigned int i_ball = 0; i_ball < balls.size(); i_ball++) {
				if(i_bonus -> bonus_type == BALL_SPEED_UP && balls[i_ball].speed < 8)	//球加速
					balls[i_ball].speed += 1;
				if(i_bonus -> bonus_type == BALL_SPEED_DOWN && balls[i_ball].speed >= 3)//球減速
					balls[i_ball].speed -= 1;
				if(i_bonus -> bonus_type == BALL_SPEED_RECOVER)							//球初速
					balls[i_ball].speed = 5;
				if(i_bonus -> bonus_type == BALL_SIZE_UP && balls[i_ball].width <= 60)	//球變大
					balls[i_ball].setSize(balls[i_ball].width*2,balls[i_ball].height*2);
				if(i_bonus -> bonus_type == BALL_SIZE_DOWN && balls[i_ball].width > 7.5)//球變小		
					balls[i_ball].setSize(balls[i_ball].width/2,balls[i_ball].height/2);
				if(i_bonus -> bonus_type == BALL_SIZE_RECOVER)							//球初始大小		
					balls[i_ball].setSize(15,15);
			}
			if(i_bonus -> bonus_type == BAR_SIZE_UP && bar.width < 500 )				//板子加長
				bar.setSize(bar.width + 50,bar.height);
			if(i_bonus -> bonus_type == BAR_SIZE_DOWN && bar.width > 50)				//板子變短
				bar.setSize(bar.width - 50,bar.height);
			if(i_bonus -> bonus_type == BALL_NUMBER_UP && balls.size() < 5) {			//球數增加 最多5個
				Ball new_ball;
				new_ball.speed = (balls.begin() -> speed);	
				new_ball.dx = cos(60 * PI/180) * new_ball.speed;	
				new_ball.dy = sin(60 * PI/180) * new_ball.speed;
				new_ball.setLocation(bar.x,bar.y + bar.height/2 + (balls.begin() -> height) + 5);	//新球位置在板子的正上方
				new_ball.setSize( (balls.begin() -> width) , (balls.begin() -> height) );			
				new_ball.setColor(0.9,0.0,0.0);
				balls.push_back(new_ball);
			}
			if(i_bonus -> bonus_type == BALL_NUMBER_INFINITE)							//球量無限　
				ball_infinite = true;
			i_bonus = bonuses.erase(i_bonus);
		}
		else {
			i_bonus++;
		}
	}
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);  //清除畫面

	for(unsigned int i = 0; i < blocks.size(); i++) {
		blocks[i].draw();	//畫block
	}
	for(unsigned int i = 0; i < balls.size(); i++) {	
		balls[i].draw();	//畫球
	}

	//板子移動
	bar.dx = 0;
	bar.dy = 0;
	int movement = 5;
	if(keyLeft)
		bar.dx = -movement;
	if(keyRight)
		bar.dx = movement;

	//版子防止出界
	if(bar.x < bar.width/2)
		bar.x = bar.width/2;
	if(bar.x > 600 - bar.width/2)
		bar.x = 600 - bar.width/2;

	bar.move();
	bar.draw();				//畫板子
	
	for(unsigned int i = 0; i < bonuses.size(); i++) {
		bonuses[i].move();	//道具移動
		bonuses[i].draw();	//畫道具
	}

	glFlush();			//清除buffer
	glutSwapBuffers();	//動畫用
}

//GAME OVER畫面
void display_GameOver(void)
{
	char clear_words[10] = "GAME OVER"; 
	int shift = 0;
	glClear(GL_COLOR_BUFFER_BIT);  //清除畫面

	glColor3f( 0.0, 0.0, 0.0); 
	glRasterPos2i(275, 400);

	for(int i = 0; i < 10; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, clear_words[i]);
		shift += glutBitmapWidth(GLUT_BITMAP_9_BY_15, clear_words[i]);
		glRasterPos2i(275 + shift, 400);
	}

	glFlush();			//清除buffer
	glutSwapBuffers();	//動畫用
}
//GAME CLEAR 畫面
void display_Clear(void)
{
	char clear_words[15] = "GAME CLEAR"; 
	int shift = 0;
	glClear(GL_COLOR_BUFFER_BIT);  //清除畫面

	glColor3f( 0.0, 0.0, 0.0); 
	glRasterPos2i(275, 400);

	for(int i = 0; i < 15; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, clear_words[i]);
		shift += glutBitmapWidth(GLUT_BITMAP_9_BY_15, clear_words[i]);
		glRasterPos2i(275 + shift, 400);
	}

	glFlush();			//清除buffer
	glutSwapBuffers();	//動畫用
}

//鍵盤事件內容
//初始化控制  Space重置 ESC結束
void keyboard(unsigned char key, int x, int y)	
{
	printf("\nkeyboard key=%c, x=%d, y=%d",key,x,y);
	if(key == ' ') {
		myinit();//初始化
		glutDisplayFunc(display);//重新註冊影格事件
	}
	if(key == 27)	//ESC的ASCLL碼
		exit(true);

	if((key == 'z' || key == 'Z') && ball_infinite == true) {
		Ball new_ball;
		new_ball.speed = (balls.begin() -> speed);	
		new_ball.dx = cos(90 * PI/180) * new_ball.speed;	
		new_ball.dy = sin(90 * PI/180) * new_ball.speed;
		new_ball.setLocation(bar.x,bar.y + bar.height/2 + (balls.begin() -> height) + 5);	//新球位置在板子的正上方
		new_ball.setSize( (balls.begin() -> width) , (balls.begin() -> height) );			
		new_ball.setColor(0.9,0.0,0.0);
		balls.push_back(new_ball);
	}
}
//板子控制　與按下ESC鍵結束遊戲
void keyboardS(int key, int x, int y)
{
	printf("\nkeyboardS key=%d, x=%d, y=%d",key,x,y);
	if(key == GLUT_KEY_RIGHT)
		keyRight = true;
	if(key == GLUT_KEY_LEFT)
		keyLeft = true;
	if(key == GLUT_KEY_F1) {	//加入按F1打開球無限  開啟狀態按F1則關閉　測試用
		if(ball_infinite)
			ball_infinite = false;
		else
			ball_infinite = true;
	}
}
void keyboardSUp(int key, int x, int y)
{
	printf("\nkeyboardSUp key=%d, x=%d, y=%d",key,x,y);
	if(key == GLUT_KEY_RIGHT)
		keyRight = false;
	if(key == GLUT_KEY_LEFT)
		keyLeft = false;
}

//動畫用
void idle(void)
{
	ball_movement();				//球的移動控制　＆　碰撞偵測：球與邊界　並含有 球與磚塊 球與板子 的Function
	Collision_Bar_and_Border();		//碰撞偵測：板子與邊界
	Collision_Bar_and_Bonus();		//碰撞偵測：板子與道具

	if(blocks.empty())	//場上沒磚塊 遊戲獲勝
		glutDisplayFunc(display_Clear);
	else if(balls.empty())	//場上沒球	 遊戲結束
		glutDisplayFunc(display_GameOver);

	Sleep(10);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	srand ( time(NULL) );	//打亂隨機

	// standard GLUT initialization 
    glutInit(&argc,argv);
    
	//設定畫圖模式
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB); //預設值
	
	glutInitWindowSize(ww,wh);	//視窗大小 600 x 800 pixel window 
    glutInitWindowPosition(0,0);	//視窗座標 place window top left on display
    glutCreateWindow("B9630126_Homework2");	//視窗標題 window title 

	//鍵盤事件 (控制起始)
	glutKeyboardFunc(keyboard);
    
	//鍵盤事件-特殊鍵 (控制板子)
	glutSpecialFunc(keyboardS);
	glutSpecialUpFunc(keyboardSUp);

	//影格事件
	glutDisplayFunc(display);

	//動畫
	glutIdleFunc(idle); 

    myinit(); 
  
    glutMainLoop(); // enter event loop
}