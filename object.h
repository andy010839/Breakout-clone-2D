
#pragma once
#include <gl/glut.h>

class Object
{
	public:
	float x;	//物件座標
	float y;

	float dx;	//向量位移
	float dy;

	float width;
	float height;
	float colorR;
	float colorG;
	float colorB;

	void draw();

	void move()
	{
		x += dx;
		y += dy;
	}

	void setLocation(float xx,float yy)
	{
		x = xx;
		y = yy;
	}
	void setSize(float w,float h)
	{
		width = w;
		height = h;
	}
	void setColor(float r,float g, float b)
	{
		colorR = r;
		colorG = g;
		colorB = b;
	}
};

//磚塊
class Block:public Object
{
	public:
	int hp;

	void setColorFromHP()
	{
		if(hp == 1)
			setColor(0.0,0.0,0.0);	//黑
		if(hp == 2)
			setColor(0.5,0.5,1.0);	//藍
		if(hp == 3)
			setColor(0.4,0.8,1.0);	//淺藍
		if(hp == 4)
			setColor(1.0,0.9,0.5);	//皮膚
		if(hp == 5)
			setColor(0.9,0.0,0.0);	//紅 沒有用到
	}

	void draw()
	{
		glPushMatrix();
			glTranslatef(x,y,0.0);
			glScalef(width,height,1.0);

			//註冊點在中央的方塊			
			glColor3f(colorR, colorG, colorB); 
			glBegin(GL_QUADS);
				glVertex2f(-0.5, -0.5);
				glVertex2f(-0.5,  0.5);
				glVertex2f( 0.5,  0.5);
				glVertex2f( 0.5, -0.5);
			glEnd();

		glPopMatrix();
	}
};

//球
class Ball:public Object
{
	public:
	float speed;

	float get_theta()	//取得球目前向量的角度 第一象限
	{
		return atan( fabs(dy) / fabs(dx) );
	} 

	void draw()
	{
		double c = 3.14159/180.0;

		glPushMatrix();
			glTranslatef(x,y,0.0);
			glScalef(width,height,1.0);

			//註冊點在中央的球			
			glColor3f(colorR, colorG, colorB); 
			glBegin(GL_TRIANGLE_FAN);
				glVertex2d(0,0);	//圓心

				for(double theta = -180.0; theta <= 180.0; theta++)
				{
					glVertex2d( cos(c*theta) /2, sin(c*theta) /2);	//圓的ｘｙ座標
				}
			glEnd();

		glPopMatrix();
	}
};

//板子
class Bar:public Object
{
	public:

	void draw()
	{
		glPushMatrix();
			glTranslatef(x,y,0.0);
			glScalef(width,height,1.0);

			//註冊點在中央的板子			
			glColor3f(colorR, colorG, colorB); 
			glBegin(GL_QUADS);
				glVertex2f(-0.5, -0.5);
				glVertex2f(-0.5,  0.5);
				glVertex2f( 0.5,  0.5);
				glVertex2f( 0.5, -0.5);
			glEnd();

		glPopMatrix();
	}
};

//道具
class Bonus:public Object
{
	public:
	int bonus_type;

	void setBonus_Type(int bt) 
	{
		bonus_type = bt;
	}

	void draw()
	{
		glPushMatrix();
		glTranslatef(x,y,0.0);
		glScalef(width,height,1.0);

		//註冊點在中央的道具			
		glColor3f(colorR, colorG, colorB); 

		switch(bonus_type) {
			case 0:	//道具種類為球的速度		矩形
			case 1:
			case 2:
				glBegin(GL_QUADS);
					glVertex2f(-0.5, -0.5);
					glVertex2f(-0.5,  0.5);
					glVertex2f( 0.5,  0.5);
					glVertex2f( 0.5, -0.5);
				glEnd();
				break;

			case 3:	//道具種類為球的大小		三角形
			case 4:
			case 5:
				glBegin(GL_TRIANGLE_STRIP);
					glVertex2f( 0.0,  0.5);
					glVertex2f( 0.5, -1.0);
					glVertex2f(-0.5, -1.0);
				glEnd();
				break;

			case 6:	//道具種類為板子變長		<-->
				glBegin(GL_TRIANGLE_STRIP);
					glVertex2f( 1.0,  0.0);
					glVertex2f( 0.5,  1.0);
					glVertex2f( 0.5, -1.0);
				glEnd();
				glBegin(GL_TRIANGLE_STRIP);
					glVertex2f(-1.0,  0.0);
					glVertex2f(-0.5,  1.0);
					glVertex2f(-0.5, -1.0);
				glEnd();
				glBegin(GL_QUADS);
					glVertex2f(-0.5, -0.5);
					glVertex2f(-0.5,  0.5);
					glVertex2f( 0.5,  0.5);
					glVertex2f( 0.5, -0.5);
				glEnd();
				break;

			case 7: //道具種類為板子變短		-><-
				glBegin(GL_TRIANGLE_STRIP);
					glVertex2f( 0.0,  0.0);
					glVertex2f( 0.5,  1.0);
					glVertex2f( 0.5, -1.0);
				glEnd();
				glBegin(GL_TRIANGLE_STRIP);
					glVertex2f( 0.0,  0.0);
					glVertex2f(-0.5,  1.0);
					glVertex2f(-0.5, -1.0);
				glEnd();
				glBegin(GL_QUADS);
					glVertex2f( 1.0,  0.5);
					glVertex2f( 1.0, -0.5);
					glVertex2f( 0.5, -0.5);
					glVertex2f( 0.5,  0.5);
				glEnd();
				glBegin(GL_QUADS);
					glVertex2f(-1.0,  0.5);
					glVertex2f(-1.0, -0.5);
					glVertex2f(-0.5, -0.5);
					glVertex2f(-0.5,  0.5);
				glEnd();
				break;

			case 8:	//道具種類為球的數量增加	六角形
			case 9:
				double alpha = 60 * 3.14159/180.0;
				glBegin(GL_POLYGON);
					for(int i = 0; i < 6; i++) {
						glVertex2d( cos(i*alpha)/2 , sin(i*alpha) );	
					}
				glEnd();				
		}
		glPopMatrix();
	}
};