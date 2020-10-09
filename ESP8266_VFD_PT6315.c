#define clk D2 //D2
#define din D1 //D1
#define stb D7 //D7
#define vfd_cmd   0x1
#define vfd_dat  0x2
uint8_t vfd_buf[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
uint32_t vfd_bs_mode;
/*
 * 写数据顺序
 * 1：写指令二：设置工作模式
 * 2：写指令三：设置起始地址
 *    随后开始发送数据，最多32位
 * 3：写指令一：设置数码管段位
 * 4：写指令四：调节亮度
 */
void ds_pt6315(uint16_t con, uint16_t len)
{
  uint8_t i, ds1, ds2, ds3, ds4, ds5, ds6;
  uint8_t pt63xx_cmd1 = 0x7;  // 16极
  uint8_t pt63xx_cmd2 = 0x40; // 工作模式，自动地址，写显示数据
  uint8_t pt63xx_cmd3 = 0xc0;// 起始地址 0
  uint8_t pt63xx_cmd4 = 0x8F; // 亮屏，亮度控制
 digitalWrite(stb, 0);
 pt6315_send(vfd_cmd, pt63xx_cmd2);
  digitalWrite(stb, 1);
  digitalWrite(stb, 0);
  pt6315_send(vfd_cmd, pt63xx_cmd3);
  for (int i = 0; i < 32; i++)
  {
    pt6315_send(vfd_dat, vfd_buf[i]);
  }
  digitalWrite(stb, 1);
  digitalWrite(stb, 0);
  pt6315_send(vfd_cmd, pt63xx_cmd1);
  digitalWrite(stb, 1);
  digitalWrite(stb, 0);
  pt6315_send(vfd_cmd, pt63xx_cmd4);
  digitalWrite(stb, 1);
}
/*
 * 驱动部分，其实就是模拟时序发送数据
 */
void pt6315_send(uint8_t cmd, uint16_t dat)
{
  uint8_t i, j;
  if (cmd == vfd_cmd)
  {
    for (int i = 0; i <= 7; i++) {
      if (dat & 1)
      {
        digitalWrite(din, 1);
      }
      else
      {
        digitalWrite(din, 0);
      }
      dat >>= 1;
      digitalWrite(clk, 1);
      digitalWrite(clk, 0);
    }
  } else if (cmd == vfd_dat) {
    for (i = 0; i <= 7; i++) {
      if (dat & 1)
      {
        digitalWrite(din, 1);
      }
      else
      {
        digitalWrite(din, 0);
      }
      dat >>= 1;
      digitalWrite(clk, 1);
      digitalWrite(clk, 0);
    }
  }
}
