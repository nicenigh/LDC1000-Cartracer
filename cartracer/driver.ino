//定义变量程序段
//把小车左轮电机编码器码盘的OUTA信号连接到Arduino控制器的数字端口2，
//数字端口2是Arduino的外部中断0的端口。
#define PinA_left 2 //外部中断0
//把小车右车轮电机编码器码盘的OUTA信号连接到Arduino控制器的数字端口3，
//数字端口3是Arduino的外部中断1的端口。
#define PinA_right 3 //外部中断1
int E_left =4; //L298P直流电机驱动板的左轮电机使能端口连接到数字接口4
int E_right =5; //连接小车右轮电机的使能端口到数字接口5
int pwm_right; //小车右轮电机的PWM功率值
int pwm_left;//上位机控制字节，用于提供给左轮电机PWM功率值。
int count1 = 0;  //左轮编码器码盘脉冲计数值
int count2= 0; //右轮编码器码盘脉冲计数值
long time1 = 0, time2 = 0; // 时间标记


void carinit()
{
  pwm_right = pwm_advance;
  pwm_left = pwm_advance;
  pinMode(E_left, OUTPUT); 
  pinMode(E_right, OUTPUT);
  pinMode(PinA_left,INPUT); 
  pinMode(PinA_right,INPUT); 
  //定义外部中断0和1的中断子程序Code(),中断触发为下跳沿触发
  //当编码器码盘的OUTA脉冲信号发生下跳沿中断时，
  //将自动调用执行中断子程序Code()。
  attachInterrupt(0, Code1, FALLING);//小车左车轮电机的编码器脉冲中断函数
  attachInterrupt(1, Code2, FALLING);//小车右车轮电机的编码器脉冲中断函数
}


float carrun(char f)
{
  String strf;
  detachInterrupt(0); // 关闭外部中断0
  detachInterrupt(1); // 关闭外部中断1    
   //把每一秒钟编码器码盘计得的脉冲数，换算为当前转速值
  float runl = (count1+count2)/2;
  if(f=='a')
  {
    strf=String("") + "direction:back   ";
//PID修复
    //根据左右车轮计数差count1-count2，乘以比例因子pid_fix，获得比例调节后的右车轮电机PWM功率值
    pwm_right=(float)pwm_right+((float)count1-(float)count2)*pid_fix;
    advance();
  }
  if(f=='b') 
  {
    strf=String("") + "direction:advance";
    back();  
  } 
  if(f=='l')
  {
    strf=String("") + "direction:left   ";
    left();
  }
  if(f=='r') 
  {
    strf=String("") + "direction:right  ";
    right();
  }
  if(f=='s') 
  {
    strf=String("") + "direction:stop   ";
    Stop(); 
  } 
    
  lcdprint(strf,2,0);
  count1 = 0;   //把脉冲计数值清零，以便计算下一秒的脉冲计数
  count2 = 0; 
  attachInterrupt(0, Code1,FALLING); // 重新开放外部中断0
  attachInterrupt(1, Code2,FALLING); // 重新开放外部中断1
  return runl;
}
  
//子程序程序段
void advance()//小车前进
{
     analogWrite(E_left,pwm_left);
     analogWrite(E_right,pwm_right);
}
void back()//小车后退
{
     analogWrite(E_left,pwm_left);
     analogWrite(E_right,pwm_right);
}
void Stop()//小车停止
{
     digitalWrite(E_left,LOW);
     digitalWrite(E_right,LOW);
} 
void left()//小车左转
{
     analogWrite(E_left,0);
     analogWrite(E_right,pwm_turn);
}
void right()//小车右转
{
     analogWrite(E_left,pwm_turn*turn_fix);
     analogWrite(E_right,0);
}

// 左侧车轮电机的编码器码盘计数中断子程序
void Code1()
{  
   //为了不计入噪音干扰脉冲，当2次中断之间的时间大于5ms时，计一次有效计数
  if((millis()-time1)>5) 
  {
    //当编码器码盘的OUTA脉冲信号下跳沿每中断一次，
    count1++; // 编码器码盘计数加一  
    time1=millis();
  }
}
// 右侧车轮电机的编码器码盘计数中断子程序
void Code2()
{  
  if((millis()-time2)>5) 
  {
    //当编码器码盘的OUTA脉冲信号下跳沿每中断一次，
    count2++; // 编码器码盘计数加一
    time2=millis();  
  }
}
