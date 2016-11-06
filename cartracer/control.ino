#define tone_pin 9//蜂鸣器引脚

int out_time = 0;//小车从线上跑出丢失信号的时间长

int line_px = 0; //线上的ldc参考值
int out_px = 0;  //线下的ldc参考值
int coin_px = 0; //硬币的ldc参考值


//f当前转向，lf1、lf2前两次回到线上的成功转向，用于转向加速
char f = 'a';
char lf1 = 'a';
char lf2 = 'a';
int lastss = 0;        //上次信号分析值

int lowsignal = 0;
int highsignal = 0;
  
int losttime = 0;      //信号丢失时间

void orginit(bool autotune)//初始化自动调参
{
  bool initsucceed = true;   //自动调参成功表识
  pinMode(tone_pin, OUTPUT); //蜂鸣器接口输出模式
  int i;
  lcdprint("initializating",0,0);
  lcdprint("... ...",1,0);

//计算线上平均值
  long proximtysum;
  for(i=0;i<100;i++)
  {
    proximtysum=proximtysum+ldcread();
    delay(10);
  }
  int orgpx=proximtysum/i;//原始线上平均值
  
  if(autotune)//自动调参
  {
    
    int maxpx=orgpx;//最大ldc值
    int minpx=orgpx;//最小ldc值
    int opxs[40];//调参时数据存放数组
  
    int turnltime;  //左转回到线上需要的时间，用于转弯pid修复
    int turnrtime;  //右转回到线上需要的时间，用于转弯pid修复
  
  //向左200ms，记录ldc最大值最小值
    carrun('l');
    for(i=0;i<20;i++)
    {
      opxs[i]=ldcread();
      if(opxs[i]>maxpx)
        maxpx=opxs[i];
      if(opxs[i]<minpx)
        minpx=opxs[i];
      delay(10);
    }
    line_px=(maxpx+orgpx)/2;       //计算线上ldc值
    out_px=minpx+(maxpx-line_px);  //计算出线ldc值
    int linepxm=(float)line_px*0.8+(float)out_px*0.2; //检测线上的值要降低一些
  
  //向右400ms，记录ldc最大值最小值
    carrun('r');
    for(i=0;i<40;i++)
    {
      opxs[i]=ldcread();
      if(opxs[i]>maxpx)
        maxpx=opxs[i];
      if(opxs[i]<minpx)
        minpx=opxs[i];
      delay(10);
    }
    line_px=(maxpx+orgpx)/2;                //计算线上ldc值
    out_px=minpx+(maxpx-orgpx)/2;           //计算出线ldc值
    for(i=0;opxs[i]>linepxm;i++); //找到回到线上的时刻
    turnrtime=i;
    turn_fix=i/20;      //转弯pid修复值
    out_time=i;         //跑出线的时间
  
  //向左找线，记录值
    carrun('l');
    i=0;
    opxs[i]=ldcread();
    while(opxs[i]>linepxm)//循环知道找到线
    {
      if(i>40)//超时向右转，调参失败
      {
        carrun('r');
        i=0;
        opxs[i]=ldcread();
        while(opxs[i]>linepxm)//循环知道找到线
        {
          opxs[i]=ldcread();
          if(opxs[i]>maxpx)
            maxpx=opxs[i];
          if(opxs[i]<minpx)
            minpx=opxs[i];
          i++;
          delay(10);
        }
        lcdprint("failured",1,0);
        initsucceed=false;
        break;
      }
      opxs[i]=ldcread();
      if(opxs[i]>maxpx)
        maxpx=opxs[i];
      if(opxs[i]<minpx)
        minpx=opxs[i];
      i++;
      delay(10);
    }
    line_px=(maxpx+orgpx)/2;  //计算线上ldc值
    out_px=minpx+(maxpx-orgpx)/2;  //计算出线ldc值
    coin_px=line_px*coinrate;    //赋硬币ldc值
  }
  else//不使用自动调参
  {
    line_px=orgpx;  //计算线上ldc值
    out_px=orgpx*outrate;  //计算出线ldc值
    coin_px=orgpx*coinrate;    //赋硬币ldc值
  }
  if(initsucceed)  //成功后修复转弯pid值和出线时间
  {
    turn_fix=(turn_fix+(40-(float)turnrtime)/(float)i)/2;
    out_time=(out_time+i)/2;
    lcdprint("succeed",1,0);
  }
  coin_px=(float)line_px*0.2+(float)coin_px*0.8; //稍降低硬币值
  out_px=(float)line_px*0.2+(float)out_px*0.8;   //稍升高出线值
  
}

char searchsignal()//计算转向，搜索线
{
  int ss=signalstatue();//检测信号
  
    if(ss<0)//跑出线，丢失信号
    {
      losttime++;//丢失信号时间+1
      
      if(losttime==1)//刚刚丢失
      {
        if(lf1=='l'&lf2=='l')//根据上次成功转向预测本次转向
          f='l';
        else if(lf1=='r'&lf2=='r')
          f='r';
        else
          f='r';
      }
      else if((losttime==5)&(ss<=lastss))//时间=50ms，并且信号在减弱
      {
        if(f=='l')//换向寻找
          f='r';
        else
          f='l';
      }
      if(ss>=lastss)//正在寻回信号，并且比上次寻信号强烈
      {
        lf2=lf1;//记录正确转向
        lf1=f;
      }
    }
    else //寻回线上
    {
      losttime=0;//丢失时间清零
      f='a';
    }
    return f;
}

int signalstatue()//计算信号状态
{
  if(now_px>coin_px) //检测到硬币
  {
    digitalWrite(tone_pin,HIGH); //打开蜂鸣器输出
    return 0;
  }
  else
  {
    digitalWrite(tone_pin,LOW); //关闭蜂鸣器输出
    if(losttime>out_time)  //丢失信号时间>出线时间
      lowsignal++;
    int l=lowsignal;
    int h=highsignal;
    lowsignal = 0;            //计数清零
    highsignal = 0;
    if(now_px<out_px) //＜出线信号
    {
      return 0-l; //返回负值降低次数
    }
    else
    {
      return h;  //返回正值升高次数
    }
  }
}

void collectsignal() //采集信号
{
  if((last_px<coin_px)&(now_px<coin_px))
  {
    if(last_px>now_px)
    {
      lowsignal++;
    }
    else
    {
      highsignal++;
    }
  }
}

