//可调节参数
extern float turn_fix = 1;       //转向PID修复参数
extern float pid_fix = 0.8;      //直行PID修复参数

extern float pwm_advance = 190;//直行pwm功率(80-255)
extern float pwm_turn = 150;   //转弯pwm功率(80-255)

extern int auto_tune_time = 20;//自动调参时小车向左转的时间=autotunet*10ms (自动调参时小车先向左转200ms，再向右转400ms，最后向左寻线，确定参数) 

bool use_auto_tune = true;//使用自动调参
bool use_kalman_filter = true;//使用卡尔曼滤波

extern float outrate = 0.98;   //丢失信号强度倍率
extern float coinrate = 1.1;   //硬币信号强度倍率

extern int carrunlenrate = 85;   //小车码盘/行驶距离倍率

//卡尔曼滤波参数
//Q:过程噪声，Q增大，动态响应变快，收敛稳定性变坏
//R:测量噪声，R增大，动态响应变慢，收敛稳定性变好
extern double ProcessNiose_Q=0.065;
extern double MeasureNoise_R=0.28;

//----------------------------------------------------------------------------------------------------------------------------------------
//固定参数
extern int last_px = 0;//上次ldc值
extern int now_px = 0;//本次ldc值


unsigned long looptime = 0, old_looptime = 0, ldc_looptime = 0, car_looptime = 0; // 时间标记

long runtime = 0; //运行时间
long runlength = 0;//运行距离


bool inited=false;//初始化完成标识

//----------------------------------------------------------------------------------------------------------------------------------------

//初始化程序段
void setup()
{

  lcdinit();//LCD屏幕初始化
  ldcinit();//ldc1000初始化
  carinit();//小车初始化

  orginit(use_auto_tune);//获取初始值，自动调参

  inited=true;//初始化完成
}


//----------------------------------------------------------------------------------------------------------------------------------------
  
//主程序段
void loop()
{
  if(inited)//初始化完成才开始循环
  {    
    looptime = millis();//获取当前时钟时间
  
    
  //读取LDC
    if(use_kalman_filter)
      ldcreadk();
    else
      ldcread();
    collectsignal();  //采集信号
  
    
  //显示时间和距离，1000ms运行一次
    if(abs(looptime - old_looptime) >= 1000)
    {
      runtime=runtime+abs(looptime - old_looptime);
      int t=runtime/1000;
      int l=runlength/carrunlenrate;
      String strt=String("") + "RunTime:  " + t + "s    ";
      String strl=String("") + "RunLength:" + l + "cm   ";
      lcdprint(strt,0,0);
      lcdprint(strl,1,0);
      old_looptime=  millis(); 
    }

  //电机控制，10ms运行一次
    if(abs(looptime - car_looptime) >= 10)
    {
      
      char f=searchsignal();//搜索信号
      
      runlength+=carrun(f);//小车运行，记录运行距离
  
  
      lcdprint(now_px,3,0);//显示当前ldc值
      car_looptime=  millis(); 
    }  
  }
}


