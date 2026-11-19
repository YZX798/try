/**
 * @file oled.c
 * @brief 波特律动OLED驱动(CH1116)
 * @anchor 波特律动(keysking 博哥在学习)
 * @version 1.0
 * @date 2023-08-04
 * @license MIT License
 *
 * @attention
 * 本驱动库针对波特律动·keysking的STM32教程学习套件进行开发
 * 在其他平台或驱动芯片上使用可能需要进行移植
 *
 * @note
 * 使用流程:
 * 1. STM32初始化IIC完成后调用OLED_Init()初始化OLED. 注意STM32启动比OLED上电快, 可等待20ms再初始化OLED
 * 2. 调用OLED_NewFrame()开始绘制新的一帧
 * 3. 调用OLED_DrawXXX()系列函数绘制图形到显存 调用OLED_Printxxx()系列函数绘制文本到显存
 * 4. 调用OLED_ShowFrame()将显存内容显示到OLED
 *
 * @note
 * 为保证中文显示正常 请将编译器的字符集设置为UTF-8
 *
 */
#include "oled.h"
#include "i2c.h"
#include <math.h>
#include <stdlib.h>
#include "main.h"

// OLED器件地址
#define OLED_ADDRESS 0x78

// I2C发送超时(ms) 原库用的是HAL_MAX_DELAY, 一旦总线异常会永久卡死在主循环里
#define OLED_I2C_TIMEOUT 5

// OLED参数
#define OLED_PAGE 8            // OLED页数
#define OLED_ROW 8 * OLED_PAGE // OLED行数
#define OLED_COLUMN 128        // OLED列数

// 显存(改成static, 避免和用户代码里的同名变量冲突)
static uint8_t OLED_GRAM[OLED_PAGE][OLED_COLUMN];

// 脏页标记: ShowFrame时只发送这些页, 没变化的页跳过
static uint8_t s_dirty[OLED_PAGE];

// ========================== 底层通信函数 ==========================

/**
 * @brief 向OLED发送数据的函数
 * @param data 要发送的数据
 * @param len 要发送的数据长度
 * @return None
 * @note 此函数是移植本驱动时的重要函数 将本驱动库移植到其他平台时应根据实际情况修改此函数
 */
void OLED_Send(uint8_t *data, uint8_t len) {
  // 用有限超时: 失败就放弃本帧, 不能让主循环卡死
  HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, data, len, OLED_I2C_TIMEOUT);
}

/**
 * @brief 向OLED发送指令
 */
void OLED_SendCmd(uint8_t cmd) {
  static uint8_t sendBuffer[2] = {0};
  sendBuffer[1] = cmd;
  OLED_Send(sendBuffer, 2);
}

/**
 * @brief 一次发送多条指令
 * @param cmds 指令数组
 * @param n 指令条数(最多15)
 * @note 控制字节0x00之后可以连续跟多条指令, 把3次事务合并成1次,
 *       刷一整屏能少发24次起始/停止信号
 */
void OLED_SendCmds(const uint8_t *cmds, uint8_t n) {
  static uint8_t buf[16];
  if (n > 15) n = 15;
  buf[0] = 0x00;
  memcpy(buf + 1, cmds, n);
  OLED_Send(buf, (uint8_t)(n + 1));
}

// ========================== OLED驱动函数 ==========================

/**
 * @brief 初始化OLED
 * @note 此函数是移植本驱动时的重要函数 将本驱动库移植到其他驱动芯片时应根据实际情况修改此函数
 */
void OLED_Init() {
  HAL_Delay(100); // STM32启动比OLED上电快, 等屏准备好再发指令(旧驱动用的是200ms)

  OLED_SendCmd(0xAE); /*关闭显示 display off*/

  OLED_SendCmd(0x00); /*设置列起始地址 set lower column address*/
  OLED_SendCmd(0x10); /*设置列结束地址 set higher column address*/

  OLED_SendCmd(0x40); /*设置起始行 set display start line*/

  OLED_SendCmd(0xB0); /*设置页地址 set page address*/

  OLED_SendCmd(0x81); /*设置对比度 contract control*/
  OLED_SendCmd(0xCF); /*128*/

  OLED_SendCmd(0xA1); /*设置分段重映射 从右到左 set segment remap*/

  OLED_SendCmd(0xA6); /*正向显示 normal / reverse*/

  OLED_SendCmd(0xA8); /*多路复用率 multiplex ratio*/
  OLED_SendCmd(0x3F); /*duty = 1/64*/

//  OLED_SendCmd(0xAD); /*设置启动电荷泵 set charge pump enable*/
//  OLED_SendCmd(0x8B); /*启动DC-DC */
//  OLED_SendCmd(0x33); /*设置泵电压 set VPP 10V */
  
  OLED_SendCmd(0x8D); /*设置启动电荷泵 set charge pump enable*/
  OLED_SendCmd(0x14); /*启动DC-DC */

  OLED_SendCmd(0xC8); /*设置输出扫描方向 COM[N-1]到COM[0] Com scan direction*/

  OLED_SendCmd(0xD3); /*设置显示偏移 set display offset*/
  OLED_SendCmd(0x00); /* 0x00 */

  OLED_SendCmd(0xD5); /*设置内部时钟频率 set osc frequency*/
  OLED_SendCmd(0xC0);

  OLED_SendCmd(0xD9); /*设置放电/预充电时间 set pre-charge period*/
  OLED_SendCmd(0x1F); /*0x22*/

  OLED_SendCmd(0xDA); /*设置引脚布局 set COM pins*/
  OLED_SendCmd(0x12);

  OLED_SendCmd(0xDB); /*设置电平 set vcomh*/
  OLED_SendCmd(0x40);

  OLED_NewFrame();
  OLED_ShowFrame();

  OLED_SendCmd(0xAF); /*开启显示 display ON*/
}

/**
 * @brief 开启OLED显示
 */
void OLED_DisPlay_On() {
  OLED_SendCmd(0x8D); // 电荷泵使能
  OLED_SendCmd(0x14); // 开启电荷泵
  OLED_SendCmd(0xAF); // 点亮屏幕
}

/**
 * @brief 关闭OLED显示
 */
void OLED_DisPlay_Off() {
  OLED_SendCmd(0x8D); // 电荷泵使能
  OLED_SendCmd(0x10); // 关闭电荷泵
  OLED_SendCmd(0xAE); // 关闭屏幕
}

/**
 * @brief 设置颜色模式 黑底白字或白底黑字
 * @param ColorMode 颜色模式COLOR_NORMAL/COLOR_REVERSED
 * @note 此函数直接设置屏幕的颜色模式
 */
void OLED_SetColorMode(OLED_ColorMode mode) {
  if (mode == OLED_COLOR_NORMAL) {
    OLED_SendCmd(0xA6); // 正常显示
  }
  if (mode == OLED_COLOR_REVERSED) {
    OLED_SendCmd(0xA7); // 反色显示
  }
}

// ========================== 显存操作函数 ==========================

/**
 * @brief 清空显存 绘制新的一帧
 */
void OLED_NewFrame() {
  memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
  memset(s_dirty, 0xFF, sizeof(s_dirty)); // 清屏后所有页都要重发
}

/**
 * @brief 将当前显存显示到屏幕上
 * @note 只发送脏页(内容有变化的页), 没变化的页直接跳过.
 *       一整页2ms左右, 只改一行文字时这里只要4ms
 */
void OLED_ShowFrame() {
  static uint8_t sendBuffer[OLED_COLUMN + 1];
  uint8_t cmds[3];
  sendBuffer[0] = 0x40;
  for (uint8_t i = 0; i < OLED_PAGE; i++) {
    if (!s_dirty[i]) continue;         // 本页没变化, 不发
    cmds[0] = 0xB0 + i;                // 设置页地址
    cmds[1] = 0x00;                    // 列地址低4位
    cmds[2] = 0x10;                    // 列地址高4位
    OLED_SendCmds(cmds, 3);
    memcpy(sendBuffer + 1, OLED_GRAM[i], OLED_COLUMN);
    OLED_Send(sendBuffer, OLED_COLUMN + 1);
    s_dirty[i] = 0;
  }
}

/**
 * @brief 设置一个像素点
 * @param x 横坐标
 * @param y 纵坐标
 * @param color 颜色
 */
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color) {
  if (x >= OLED_COLUMN || y >= OLED_ROW) return;
  if (!color) {
    OLED_GRAM[y / 8][x] |= 1 << (y % 8);
  } else {
    OLED_GRAM[y / 8][x] &= ~(1 << (y % 8));
  }
  s_dirty[y / 8] = 1;
}

/**
 * @brief 设置显存中一字节数据的某几位
 * @param page 页地址
 * @param column 列地址
 * @param data 数据
 * @param start 起始位
 * @param end 结束位
 * @param color 颜色
 * @note 此函数将显存中的某一字节的第start位到第end位设置为与data相同
 * @note start和end的范围为0-7, start必须小于等于end
 * @note 此函数与OLED_SetByte_Fine的区别在于此函数只能设置显存中的某一真实字节
 */
void OLED_SetByte_Fine(uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end, OLED_ColorMode color) {
  static uint8_t temp;
  if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
  if (color) data = ~data;

  temp = data | (0xff << (end + 1)) | (0xff >> (8 - start));
  OLED_GRAM[page][column] &= temp;
  temp = data & ~(0xff << (end + 1)) & ~(0xff >> (8 - start));
  OLED_GRAM[page][column] |= temp;
  s_dirty[page] = 1;
  // 使用OLED_SetPixel实现
  // for (uint8_t i = start; i <= end; i++) {
  //   OLED_SetPixel(column, page * 8 + i, !((data >> i) & 0x01));
  // }
}

/**
 * @brief 设置显存中的一字节数据
 * @param page 页地址
 * @param column 列地址
 * @param data 数据
 * @param color 颜色
 * @note 此函数将显存中的某一字节设置为data的值
 */
void OLED_SetByte(uint8_t page, uint8_t column, uint8_t data, OLED_ColorMode color) {
  if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
  if (color) data = ~data;
  OLED_GRAM[page][column] = data;
  s_dirty[page] = 1;
}

/**
 * @brief 设置显存中的一字节数据的某几位
 * @param x 横坐标
 * @param y 纵坐标
 * @param data 数据
 * @param len 位数
 * @param color 颜色
 * @note 此函数将显存中从(x,y)开始向下数len位设置为与data相同
 * @note len的范围为1-8
 * @note 此函数与OLED_SetByte_Fine的区别在于此函数的横坐标和纵坐标是以像素为单位的, 可能出现跨两个真实字节的情况(跨页)
 */
void OLED_SetBits_Fine(uint8_t x, uint8_t y, uint8_t data, uint8_t len, OLED_ColorMode color) {
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  if (bit + len > 8) {
    OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
    OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, len + bit - 1 - 8, color);
  } else {
    OLED_SetByte_Fine(page, x, data << bit, bit, bit + len - 1, color);
  }
  // 使用OLED_SetPixel实现
  // for (uint8_t i = 0; i < len; i++) {
  //   OLED_SetPixel(x, y + i, !((data >> i) & 0x01));
  // }
}

/**
 * @brief 设置显存中一字节长度的数据
 * @param x 横坐标
 * @param y 纵坐标
 * @param data 数据
 * @param color 颜色
 * @note 此函数将显存中从(x,y)开始向下数8位设置为与data相同
 * @note 此函数与OLED_SetByte的区别在于此函数的横坐标和纵坐标是以像素为单位的, 可能出现跨两个真实字节的情况(跨页)
 */
void OLED_SetBits(uint8_t x, uint8_t y, uint8_t data, OLED_ColorMode color) {
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
  if (bit) {
    OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, bit - 1, color);
  }
}

/**
 * @brief 设置一块显存区域
 * @param x 起始横坐标
 * @param y 起始纵坐标
 * @param data 数据的起始地址
 * @param w 宽度
 * @param h 高度
 * @param color 颜色
 * @note 此函数将显存中从(x,y)开始的w*h个像素设置为data中的数据
 * @note data的数据应该采用列行式排列
 */
void OLED_SetBlock(uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color) {
  uint8_t fullRow = h / 8; // 完整的行数
  uint8_t partBit = h % 8; // 不完整的字节中的有效位数
  for (uint8_t i = 0; i < w; i++) {
    for (uint8_t j = 0; j < fullRow; j++) {
      OLED_SetBits(x + i, y + j * 8, data[i + j * w], color);
    }
  }
  if (partBit) {
    uint16_t fullNum = w * fullRow; // 完整的字节数
    for (uint8_t i = 0; i < w; i++) {
      OLED_SetBits_Fine(x + i, y + (fullRow * 8), data[fullNum + i], partBit, color);
    }
  }
  // 使用OLED_SetPixel实现
  // for (uint8_t i = 0; i < w; i++) {
  //   for (uint8_t j = 0; j < h; j++) {
  //     for (uint8_t k = 0; k < 8; k++) {
  //       if (j * 8 + k >= h) break; // 防止越界(不完整的字节
  //       OLED_SetPixel(x + i, y + j * 8 + k, !((data[i + j * w] >> k) & 0x01));
  //     }
  //   }
  // }
}

// ========================== 图形绘制函数 ==========================
/**
 * @brief 绘制一条线段
 * @param x1 起始点横坐标
 * @param y1 起始点纵坐标
 * @param x2 终止点横坐标
 * @param y2 终止点纵坐标
 * @param color 颜色
 * @note 此函数使用Bresenham算法绘制线段
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color) {
  static uint8_t temp = 0;
  if (x1 == x2) {
    if (y1 > y2) {
      temp = y1;
      y1 = y2;
      y2 = temp;
    }
    for (uint8_t y = y1; y <= y2; y++) {
      OLED_SetPixel(x1, y, color);
    }
  } else if (y1 == y2) {
    if (x1 > x2) {
      temp = x1;
      x1 = x2;
      x2 = temp;
    }
    for (uint8_t x = x1; x <= x2; x++) {
      OLED_SetPixel(x, y1, color);
    }
  } else {
    // Bresenham直线算法
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    int16_t ux = ((dx > 0) << 1) - 1;
    int16_t uy = ((dy > 0) << 1) - 1;
    int16_t x = x1, y = y1, eps = 0;
    dx = abs(dx);
    dy = abs(dy);
    if (dx > dy) {
      for (x = x1; x != x2; x += ux) {
        OLED_SetPixel(x, y, color);
        eps += dy;
        if ((eps << 1) >= dx) {
          y += uy;
          eps -= dx;
        }
      }
    } else {
      for (y = y1; y != y2; y += uy) {
        OLED_SetPixel(x, y, color);
        eps += dx;
        if ((eps << 1) >= dy) {
          x += ux;
          eps -= dy;
        }
      }
    }
  }
}

/**
 * @brief 绘制一个矩形
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param w 矩形宽度
 * @param h 矩形高度
 * @param color 颜色
 */
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color) {
  OLED_DrawLine(x, y, x + w, y, color);
  OLED_DrawLine(x, y + h, x + w, y + h, color);
  OLED_DrawLine(x, y, x, y + h, color);
  OLED_DrawLine(x + w, y, x + w, y + h, color);
}

/**
 * @brief 绘制一个填充矩形
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param w 矩形宽度
 * @param h 矩形高度
 * @param color 颜色
 */
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color) {
  for (uint8_t i = 0; i < h; i++) {
    OLED_DrawLine(x, y+i, x+w-1, y+i, color);
  }
}

/**
 * @brief 绘制一个三角形
 * @param x1 第一个点横坐标
 * @param y1 第一个点纵坐标
 * @param x2 第二个点横坐标
 * @param y2 第二个点纵坐标
 * @param x3 第三个点横坐标
 * @param y3 第三个点纵坐标
 * @param color 颜色
 */
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color) {
  OLED_DrawLine(x1, y1, x2, y2, color);
  OLED_DrawLine(x2, y2, x3, y3, color);
  OLED_DrawLine(x3, y3, x1, y1, color);
}

/**
 * @brief 绘制一个填充三角形
 * @param x1 第一个点横坐标
 * @param y1 第一个点纵坐标
 * @param x2 第二个点横坐标
 * @param y2 第二个点纵坐标
 * @param x3 第三个点横坐标
 * @param y3 第三个点纵坐标
 * @param color 颜色
 */
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color) {
  uint8_t a = 0, b = 0, y = 0, last = 0;
  if (y1 > y2) {
    a = y2;
    b = y1;
  } else {
    a = y1;
    b = y2;
  }
  y = a;
  for (; y <= b; y++) {
    if (y <= y3) {
      OLED_DrawLine(x1 + (y - y1) * (x2 - x1) / (y2 - y1), y, x1 + (y - y1) * (x3 - x1) / (y3 - y1), y, color);
    } else {
      last = y - 1;
      break;
    }
  }
  for (; y <= b; y++) {
    OLED_DrawLine(x2 + (y - y2) * (x3 - x2) / (y3 - y2), y, x1 + (y - last) * (x3 - x1) / (y3 - last), y, color);
  }
}

/**
 * @brief 绘制一个圆
 * @param x 圆心横坐标
 * @param y 圆心纵坐标
 * @param r 圆半径
 * @param color 颜色
 * @note 此函数使用Bresenham算法绘制圆
 */
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    OLED_SetPixel(x - b, y - a, color);
    OLED_SetPixel(x + b, y - a, color);
    OLED_SetPixel(x - a, y + b, color);
    OLED_SetPixel(x - b, y - a, color);
    OLED_SetPixel(x - a, y - b, color);
    OLED_SetPixel(x + b, y + a, color);
    OLED_SetPixel(x + a, y - b, color);
    OLED_SetPixel(x + a, y + b, color);
    OLED_SetPixel(x - b, y + a, color);
    a++;
    if (di < 0) {
      di += 4 * a + 6;
    } else {
      di += 10 + 4 * (a - b);
      b--;
    }
    OLED_SetPixel(x + a, y + b, color);
  }
}

/**
 * @brief 绘制一个填充圆
 * @param x 圆心横坐标
 * @param y 圆心纵坐标
 * @param r 圆半径
 * @param color 颜色
 * @note 此函数使用Bresenham算法绘制圆
 */
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    for (int16_t i = x - b; i <= x + b; i++) {
      OLED_SetPixel(i, y + a, color);
      OLED_SetPixel(i, y - a, color);
    }
    for (int16_t i = x - a; i <= x + a; i++) {
      OLED_SetPixel(i, y + b, color);
      OLED_SetPixel(i, y - b, color);
    }
    a++;
    if (di < 0) {
      di += 4 * a + 6;
    } else {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
}

/**
 * @brief 绘制一个椭圆
 * @param x 椭圆中心横坐标
 * @param y 椭圆中心纵坐标
 * @param a 椭圆长轴
 * @param b 椭圆短轴
 */
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color) {
  int xpos = 0, ypos = b;
  int a2 = a * a, b2 = b * b;
  int d = b2 + a2 * (0.25 - b);
  while (a2 * ypos > b2 * xpos) {
    OLED_SetPixel(x + xpos, y + ypos, color);
    OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color);
    OLED_SetPixel(x - xpos, y - ypos, color);
    if (d < 0) {
      d = d + b2 * ((xpos << 1) + 3);
      xpos += 1;
    } else {
      d = d + b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2);
      xpos += 1, ypos -= 1;
    }
  }
  d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
  while (ypos > 0) {
    OLED_SetPixel(x + xpos, y + ypos, color);
    OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color);
    OLED_SetPixel(x - xpos, y - ypos, color);
    if (d < 0) {
      d = d + b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3);
      xpos += 1, ypos -= 1;
    } else {
      d = d + a2 * (-(ypos << 1) + 3);
      ypos -= 1;
    }
  }
}

/**
 * @brief 绘制一张图片
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param img 图片
 * @param color 颜色
 */
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color) {
  OLED_SetBlock(x, y, img->data, img->w, img->h, color);
}

// ================================ 文字绘制 ================================

/**
 * @brief 绘制一个ASCII字符
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param ch 字符
 * @param font 字体
 * @param color 颜色
 */
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color) {
  OLED_SetBlock(x, y, font->chars + (ch - ' ') * (((font->h + 7) / 8) * font->w), font->w, font->h, color);
}

/**
 * @brief 绘制一个ASCII字符串
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param str 字符串
 * @param font 字体
 * @param color 颜色
 */
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color) {
  uint8_t x0 = x;
  while (*str) {
    OLED_PrintASCIIChar(x0, y, *str, font, color);
    x0 += font->w;
    str++;
  }
}

/**
 * @brief 获取UTF-8编码的字符长度
 */
uint8_t _OLED_GetUTF8Len(char *string) {
  if ((string[0] & 0x80) == 0x00) {
    return 1;
  } else if ((string[0] & 0xE0) == 0xC0) {
    return 2;
  } else if ((string[0] & 0xF0) == 0xE0) {
    return 3;
  } else if ((string[0] & 0xF8) == 0xF0) {
    return 4;
  }
  return 0;
}

/**
 * @brief 绘制字符串
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param str 字符串
 * @param font 字体
 * @param color 颜色
 *
 * @note 为保证字符串中的中文会被自动识别并绘制, 需:
 * 1. 编译器字符集设置为UTF-8
 * 2. 使用波特律动LED取模工具生成字模(https://led.baud-dance.com)
 */
/**
 * @brief 绘制字符串
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param str 字符串
 * @param font 字体
 * @param color 颜色
 *
 * @note 为保证字符串中的中文会被自动识别并绘制, 需:
 * 1. 编译器字符集设置为UTF-8
 * 2. 使用波特律动LED取模工具生成字模(https://led.baud-dance.com)
 */
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color) {
  uint16_t i = 0;                                       // 字符串索引
  uint8_t oneLen = (((font->h + 7) / 8) * font->w) + 4; // 一个字模占多少字节
  uint8_t found;                                        // 是否找到字模
  uint8_t utf8Len;                                      // UTF-8编码长度
  uint8_t *head;                                        // 字模头指针
  while (str[i]) {
    found = 0;
    utf8Len = _OLED_GetUTF8Len(str + i);
    if (utf8Len == 0) break; // 有问题的UTF-8编码

    // 寻找字符  TODO 优化查找算法, 二分查找或者hash
    for (uint8_t j = 0; j < font->len; j++) {
      head = (uint8_t *)(font->chars) + (j * oneLen);
      if (memcmp(str + i, head, utf8Len) == 0) {
        OLED_SetBlock(x, y, head + 4, font->w, font->h, color);
        // 移动光标
        x += font->w;
        i += utf8Len;
        found = 1;
        break;
      }
    }

    // 若未找到字模,且为ASCII字符, 则缺省显示ASCII字符
    if (found == 0) {
      if (utf8Len == 1) {
        OLED_PrintASCIIChar(x, y, str[i], font->ascii, color);
        // 移动光标
        x += font->ascii->w;
        i += utf8Len;
      } else {
        OLED_PrintASCIIChar(x, y, ' ', font->ascii, color);
        x += font->ascii->w;
        i += utf8Len;
      }
    }
  }
}

// ==================== 与江协旧驱动兼容的接口 ====================
// 旧驱动的 y 参数是"页号"(0/2/4/6), 新驱动是"像素", 这里统一按 y * 8 换算.

/**
 * @brief 按字号取ASCII字体
 * @param size 8/12/16, 其它值按16处理
 */
static const ASCIIFont *OLED_GetASCIIFont(uint8_t size) {
  if (size == 8) return &afont8x6;
  if (size == 12) return &afont12x6;
  return &afont16x8;
}

/**
 * @brief 填充矩形(只操作显存, 不发屏)
 * @param color OLED_COLOR_REVERSED 为擦除(全黑), OLED_COLOR_NORMAL 为全亮
 */
static void OLED_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color) {
  if (w == 0 || h == 0) return;
  if ((y % 8) == 0 && (h % 8) == 0) {   // 整页对齐, 直接按字节填
    uint8_t page = y / 8;
    uint8_t n = h / 8;
    for (uint8_t p = 0; p < n; p++) {
      if (page + p >= OLED_PAGE) break;
      for (uint8_t i = 0; i < w; i++) {
        if (x + i >= OLED_COLUMN) break;
        OLED_GRAM[page + p][x + i] = color ? 0x00 : 0xFF;
      }
      s_dirty[page + p] = 1;
    }
  } else {                              // 非整页对齐, 逐像素
    for (uint8_t i = 0; i < w; i++)
      for (uint8_t j = 0; j < h; j++)
        OLED_SetPixel(x + i, y + j, color);
  }
}

/** @brief 清屏(等价旧驱动的 OLED_Clear) */
void OLED_Clear(void) {
  OLED_NewFrame();
  OLED_ShowFrame();
}

/** @brief 开显示(兼容旧驱动的函数名) */
void OLED_Display_On(void) { OLED_DisPlay_On(); }

/** @brief 关显示(兼容旧驱动的函数名) */
void OLED_Display_Off(void) { OLED_DisPlay_Off(); }

/**
 * @brief 显示一个字符(兼容旧驱动)
 * @param y 页号 0-7
 * @param size 8/12/16
 */
void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size) {
  const ASCIIFont *font = OLED_GetASCIIFont(size);
  if (x >= OLED_COLUMN) return;
  OLED_FillRect(x, y * 8, font->w, font->h, OLED_COLOR_REVERSED);
  OLED_PrintASCIIChar(x, y * 8, chr, font, OLED_COLOR_NORMAL);
  OLED_ShowFrame();
}

/**
 * @brief 显示字符串(兼容旧驱动)
 * @param x 横坐标(像素)
 * @param y 页号, 16号字占2页(0/2/4/6)
 * @param size 8/12/16
 * @note 会先擦掉字符串覆盖的区域, 所以字符串变短时不会留残影
 */
void OLED_ShowString(uint8_t x, uint8_t y, char *str, uint8_t size) {
  const ASCIIFont *font = OLED_GetASCIIFont(size);
  uint16_t w;

  if (x >= OLED_COLUMN) return;

  w = (uint16_t)strlen(str) * font->w;
  if (w > (uint16_t)(OLED_COLUMN - x)) w = OLED_COLUMN - x;

  OLED_FillRect(x, y * 8, (uint8_t)w, font->h, OLED_COLOR_REVERSED);
  OLED_PrintASCIIString(x, y * 8, str, font, OLED_COLOR_NORMAL);
  OLED_ShowFrame();
}

/**
 * @brief 显示无符号数字(兼容旧驱动)
 * @param len 显示位数, 不足左侧补空格
 * @param size 8/12/16
 */
void OLED_ShowNum(uint8_t x, uint8_t y, unsigned int num, uint8_t len, uint8_t size) {
  const ASCIIFont *font = OLED_GetASCIIFont(size);
  char buf[11];
  uint8_t n = 0;

  if (x >= OLED_COLUMN) return;
  if (len > 10) len = 10;

  do {
    buf[n++] = (char)('0' + (num % 10));
    num /= 10;
  } while (num && n < len);

  OLED_FillRect(x, y * 8, (uint8_t)(len * font->w), font->h, OLED_COLOR_REVERSED);
  for (uint8_t i = 0; i < len; i++) {
    char c = (i < n) ? buf[n - 1 - i] : ' ';
    OLED_PrintASCIIChar(x + i * font->w, y * 8, c, font, OLED_COLOR_NORMAL);
  }
  OLED_ShowFrame();
}


//缓冲区
char display_buf[20];

void OLED_Show_T2(void)
{
	sprintf(display_buf, "|%-2s|%-5s|%-4s|", g_State.Mode_TaskName(&g_State), g_State.Mode_RunName(&g_State), g_State.Mode_TuneName(&g_State));
	OLED_ShowString(0,0,display_buf,16);
	sprintf(display_buf,"%s:%.2f",g_State.Mode_TuneName(&g_State),g_State.tuner->Show);
	OLED_ShowString(0,2,display_buf,16);
	sprintf(display_buf,"roll:%.2f",g_mpu.roll);
	OLED_ShowString(0,4,display_buf,16);
	sprintf(display_buf, "PWM%4d,%6d", (int)PWM_L_OUT, (int)PWM_R_OUT);
	OLED_ShowString(0, 6, display_buf, 16);
}

void OLED_Show_T1(void)
{
	sprintf(display_buf, "|%-2s|%-5s|%-4s|",
	        g_State.Mode_TaskName(&g_State),
	        g_State.Mode_RunName(&g_State),
	        g_State.Mode_TuneName(&g_State));
	OLED_ShowString(0, 0, display_buf, 16);

	sprintf(display_buf, "%s:%.2f",
	        g_State.Mode_TuneName(&g_State), g_State.tuner->Show);
	OLED_ShowString(0, 2, display_buf, 16);

	sprintf(display_buf, "T:%4d V:%4d", (int)g_State.tgt, (int)g_State.real_V);
	OLED_ShowString(0, 4, display_buf, 16);

	sprintf(display_buf, "PWM:%4d,%6d", (int)PWM_L_OUT, (int)PWM_R_OUT);
	OLED_ShowString(0, 6, display_buf, 16);
}
