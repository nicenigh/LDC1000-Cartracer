//LDC1000模块，已封装好
#include <SPI.h>


double x_last=0;
double p_last=0;


//ldc1000 所需参数，不用调
#define LDC1000_CMD_REVID     0x00
#define LDC1000_CMD_RPMAX     0x01
#define LDC1000_CMD_RPMIN     0x02
#define LDC1000_CMD_SENSORFREQ  0x03
#define LDC1000_CMD_LDCCONFIG   0x04
#define LDC1000_CMD_CLKCONFIG   0x05
#define LDC1000_CMD_THRESHILSB  0x06
#define LDC1000_CMD_THRESHIMSB  0x07
#define LDC1000_CMD_THRESLOLSB  0x08
#define LDC1000_CMD_THRESLOMSB  0x09
#define LDC1000_CMD_INTCONFIG   0x0A
#define LDC1000_CMD_PWRCONFIG   0x0B
#define LDC1000_CMD_STATUS    0x20
#define LDC1000_CMD_PROXLSB   0x21
#define LDC1000_CMD_PROXMSB   0x22
#define LDC1000_CMD_FREQCTRLSB  0x23
#define LDC1000_CMD_FREQCTRMID  0x24
#define LDC1000_CMD_FREQCTRMSB  0x25

#define RPMAX   0x13        /**< maximum calibration value for RPMAX */
#define RPMIN   0x3A        /**< minimum calibration value for RPMAX */

unsigned char proximtyData[2];
unsigned char frequencyData[3];

unsigned int proximtyDataMAX;
unsigned int frequencyDataMAX;

unsigned int proximtyDataTEMP;
unsigned int frequencyDataTEMP;

const int chipSelectPin = 53;

static byte txlen;
static byte rxlen;
static byte *txbuf;
static byte *rxbuf;
static byte txaddr;
static byte wordbuf;


void ldcinit() {
   byte orgVal[20];
   int i;
  // set the slaveSelectPin as an output:
  pinMode (chipSelectPin, OUTPUT);
  // initialize SPI:
   Serial.begin(9600);

  // start the SPI library:
  SPI.begin();
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  pinMode(chipSelectPin, OUTPUT);
  delay(100);
        spi_writeByte(LDC1000_CMD_RPMAX,       RPMAX);
        spi_writeByte(LDC1000_CMD_RPMIN,       RPMIN);
  spi_writeByte(LDC1000_CMD_SENSORFREQ,  0x94);
  spi_writeByte(LDC1000_CMD_LDCCONFIG,   0x17);
  spi_writeByte(LDC1000_CMD_CLKCONFIG,   0x02);
  spi_writeByte(LDC1000_CMD_INTCONFIG,   0x02);

  spi_writeByte(LDC1000_CMD_THRESHILSB,  0x50);
  spi_writeByte(LDC1000_CMD_THRESHIMSB,  0x14);
  spi_writeByte(LDC1000_CMD_THRESLOLSB,  0xC0);
  spi_writeByte(LDC1000_CMD_THRESLOMSB,  0x12);

  spi_writeByte(LDC1000_CMD_PWRCONFIG,   0x01);

  //read all registers

    spi_readBytes(LDC1000_CMD_REVID, &orgVal[0],12);
}

byte spi_writeByte(byte addr, byte data)
{
    wordbuf = data;          // copy from stack to memory
    txbuf = &wordbuf;
    txaddr = addr & ~0x80;
    digitalWrite(chipSelectPin,LOW); //P1OUT &= ~BIT0;
    SPI.transfer(txaddr);//while (!(IFG2&UCA0TXIFG));
    //UCA0TXBUF = txaddr;
    //while (!(IFG2&UCA0TXIFG));
    //UCA0TXBUF = *txbuf;
                SPI.transfer(data);
    digitalWrite(chipSelectPin,HIGH); //P1OUT |= BIT0;
    return 0;
}

byte spi_readByte( byte addr, byte data)
{
      //rxbuf = data;
    txaddr = addr | 0x80;
    digitalWrite(chipSelectPin,LOW);//P1OUT &= ~BIT0;
    SPI.transfer(txaddr);//while (!(IFG2&UCA0TXIFG));
    //UCA0TXBUF = txaddr;
    //while (!(IFG2&UCA0TXIFG));
    //UCA0TXBUF = 0;
    //while (UCA0STAT & UCBUSY);
    data = SPI.transfer(0x00);//* rxbuf = UCA0RXBUF;
    //while (UCA0STAT & UCBUSY);
    digitalWrite(chipSelectPin,HIGH);//P1OUT |= BIT0;
    return 0;
}
byte spi_readBytes( byte addr, byte * buffer, byte len)
{
        int rxlen;    
        rxlen = len;
    rxbuf = buffer;
    txaddr = addr | 0x80;
    digitalWrite(chipSelectPin,LOW);//P1OUT &= ~BIT0;
    //while (!(IFG2&UCA0TXIFG));
    SPI.transfer(txaddr);//UCA0TXBUF = txaddr;
    while (rxlen > 0) {
        //SPI.transfer(0x00);
        *rxbuf = SPI.transfer(0x00);
      rxbuf++;
      rxlen--;
      }
                digitalWrite(chipSelectPin,HIGH);
    return 0;
}

int ldcspiread()
{
  spi_readBytes(LDC1000_CMD_PROXLSB,&proximtyData[0],2);
  spi_readBytes(LDC1000_CMD_FREQCTRLSB,&frequencyData[0],3);
  proximtyDataTEMP =  ((unsigned char)proximtyData[1]<<8) + proximtyData [0];
  frequencyDataTEMP = ((unsigned char)frequencyData[1]<<8) + frequencyData[0];
  return (int)proximtyDataTEMP;
}

//读取ldc1000的值
int ldcread() {
  last_px=now_px;
  now_px=ldcspiread();
  return now_px;
}

//读取滤波后ldc1000的值
int ldcreadK() {
  if(p_last==0)
  {
    p_last=ldcspiread();
  }
  
  last_px=now_px;
  now_px=KalmanFilter(ldcspiread());
  return now_px;//返回当前数据
}



int KalmanFilter(int ResrcData)
{
  double R = MeasureNoise_R;
  double Q = ProcessNiose_Q;


  double x_mid = x_last;
  double x_now;

  double p_mid ;
  double p_now;
  double kg;        

  x_mid=x_last; //x_last=x(k-1|k-1),x_mid=x(k|k-1)
  p_mid=p_last+Q; //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=噪声
  kg=p_mid/(p_mid+R); //kg为kalman filter，R为噪声
  x_now=x_mid+kg*(ResrcData-x_mid);//估计出的最优值
          
  p_now=(1-kg)*p_mid;//最优值对应的covariance        

  p_last = p_now; //更新covariance值
  x_last = x_now; //更新系统状态值

  return (int)x_now;                
}
