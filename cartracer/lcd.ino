//LCD屏幕模块，已封装好

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the  address to 0x27 for a 16 chars and 2 line display

void lcdinit()
{
    lcd.init();
    lcd.backlight();
    lcd.home();
}

void lcdprint(char q,int l,int c)
{
  lcd.setCursor(c, l);   
  lcd.print(q);
}
void lcdprint(float q,int l,int c)
{
  String str = String("") + q;
  lcdprint(str, l, c);
}
void lcdprint(int q,int l,int c)
{
  String str = String("") + q;
  lcdprint(str, l, c);
}
void lcdprint(unsigned int q,int l,int c)
{
  lcdprint((int)q,l,c);
}
void lcdprint(String q,int l,int c)
{
  lcd.setCursor(c, l);   
  for(int i=0;i<q.length();i++)
  {
    lcd.print(q.charAt(i));
  }
}    
void lcdprint(String q)
{
  lcd.print(q);
}
