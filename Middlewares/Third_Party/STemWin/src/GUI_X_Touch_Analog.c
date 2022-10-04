#include "GUI.h"
#include "touch.h"

void GUI_TOUCH_X_ActivateX(void) 
{

}

void GUI_TOUCH_X_ActivateY(void)
{

}

//获取X坐标AD值
int  GUI_TOUCH_X_MeasureX(void) 
{
	return	tp_read_value_x();
}

//获取Y坐标AD值
int  GUI_TOUCH_X_MeasureY(void) 
{	
	return tp_read_value_y();
}