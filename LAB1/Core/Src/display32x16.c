/*
 * display32x16.c
 *
 *  Created on: 15 окт. 2023 г.
 *      Author: Niko
 */
/*
#include "disp1color.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <locale.h>

// Размеры экрана в пикселях (может вклчать несколько матриц)
uint8_t DISPLAY_Height = 16;
uint8_t DISPLAY_Width = 32;

// Размеры экрана в пикселях (может вклчать несколько матриц)
uint8_t DMD_1COLOR_ScreenHeight = 16;
uint8_t DMD_1COLOR_ScreenWidth = 32;

uint8_t DMD_1COLOR_MatrixBuff[(DMD_1COLOR_MatrixHeight >> 3) * DMD_1COLOR_MatrixWidth]; //копия буфера матрицы графич. библ. (вертикальные байты)
uint8_t DMD_1COLOR_ScreenPart = 0;
uint8_t DMD_1COLOR_Buff[4][(DISP1COLOR_Width >> 3) * (DISP1COLOR_Height >> 2)]; //буфер матрицы конченый (горизонтальные байты по строкам)


uint8_t disp1color_buff[(DISP1COLOR_Width * DISP1COLOR_Height) / 8]; //буфер графической библиотеки (вертикальные байты)

uint8_t data1[16], data2[16], data3[16], data4[16];

//==============================================================================
// Процедура заполняет буфер кадра значением FillValue
//==============================================================================
void disp1color_FillScreenbuff(uint8_t FillValue)
{
  memset(disp1color_buff, FillValue, sizeof(disp1color_buff));
}
//==============================================================================


//==============================================================================
// Процедура обновляет состояние индикаторов в соответствии с буфером кадра disp1color_buff
//==============================================================================
void disp1color_UpdateFromBuff(void)
{
  DMD_1COLOR_DisplayFullUpdate(disp1color_buff, sizeof(disp1color_buff));

  //printf("BUF horz:\n");
  //print_buf(DMD_1COLOR_Buff, sizeof(DMD_1COLOR_Buff));
}
//==============================================================================

//==============================================================================
// Процедура устанавливает состояние 1 пикселя дисплея
//==============================================================================
void disp1color_DrawPixel(int16_t X, int16_t Y, uint8_t State)
{
  //Х и У координаты пикселя, отсчитываются слева направо (Х) сверху вниз (У) с 0
  // Проверяем, находится ли точка в поле отрисовки дисплея
  if ((X >= DISP1COLOR_Width) || (Y >= DISP1COLOR_Height) || (X < 0) || (Y < 0))
    return;

  //Байты относительно матрицы расположены вертикально, в 2 ряда
  uint16_t ByteIdx = Y >> 3;
  uint8_t BitIdx = Y - (ByteIdx << 3); // Высота относительно строки байт (0<=Y<=7)
  ByteIdx *= DISP1COLOR_Width;
  ByteIdx += X;

  //Заполнение буфера данных для отрисовки
  if (State)
    disp1color_buff[ByteIdx] |= (1 << BitIdx);
  else
    disp1color_buff[ByteIdx] &= ~(1 << BitIdx);

  uint8_t buf = disp1color_buff[ByteIdx];
}
//==============================================================================


//==============================================================================
// Процедура рисует прямую линию в буфере кадра дисплея
//==============================================================================
void disp1color_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  const int16_t deltaX = abs(x2 - x1);
  const int16_t deltaY = abs(y2 - y1);
  const int16_t signX = x1 < x2 ? 1 : -1;
  const int16_t signY = y1 < y2 ? 1 : -1;

  int16_t error = deltaX - deltaY;

  disp1color_DrawPixel(x2, y2, 1);

  while (x1 != x2 || y1 != y2)
  {
    disp1color_DrawPixel(x1, y1, 1);
    const int16_t error2 = error * 2;

    if (error2 > -deltaY)
    {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX)
    {
      error += deltaX;
      y1 += signY;
    }
  }
}
//==============================================================================


//==============================================================================
// Процедура рисует прямоугольник в буфере кадра дисплея
//==============================================================================
void disp1color_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  disp1color_DrawLine(x1, y1, x1, y2);
  disp1color_DrawLine(x2, y1, x2, y2);
  disp1color_DrawLine(x1, y1, x2, y1);
  disp1color_DrawLine(x1, y2, x2, y2);
}
//==============================================================================


//==============================================================================
// Процедура рисует окружность в буфере кадра дисплея. x0 и y0 - координаты центра окружности
//==============================================================================
void disp1color_DrawCircle(int16_t x0, int16_t y0, int16_t radius)
{
  int x = 0;
  int y = radius;
  int delta = 1 - 2 * radius;
  int error = 0;

  while (y >= 0)
  {
    disp1color_DrawPixel(x0 + x, y0 + y, 1);
    disp1color_DrawPixel(x0 + x, y0 - y, 1);
    disp1color_DrawPixel(x0 - x, y0 + y, 1);
    disp1color_DrawPixel(x0 - x, y0 - y, 1);
    error = 2 * (delta + y) - 1;

    if (delta < 0 && error <= 0)
    {
      ++x;
      delta += 2 * x + 1;
      continue;
    }

    error = 2 * (delta - x) - 1;

    if (delta > 0 && error > 0)
    {
      --y;
      delta += 1 - 2 * y;
      continue;
    }

    ++x;
    delta += 2 * (x - y);
    --y;
  }
}
//==============================================================================

//==============================================================================
// Процедура подготавливает буфер на передачу в экран в соответствии с буфером pBuff графической бибилиотеки
//==============================================================================
void DMD_1COLOR_DisplayFullUpdate(uint8_t *pBuff, uint16_t BuffLen)
{
	DMD_1COLOR_UpdateRow4(0, pBuff, BuffLen);
	DMD_1COLOR_UpdateRow4(1, pBuff, BuffLen);
	DMD_1COLOR_UpdateRow4(2, pBuff, BuffLen);
	DMD_1COLOR_UpdateRow4(3, pBuff, BuffLen);
}
//==============================================================================


//==============================================================================
// Процедура обновляет поочерёдно все матрицы экрана
//==============================================================================
void DMD_1COLOR_UpdateRow4(uint8_t Row4, uint8_t *pBuff, uint16_t BuffLen)
{
	// Подготавливаем данные для передачи
	uint16_t MatrixInRow = DMD_1COLOR_ScreenWidth / DMD_1COLOR_MatrixWidth;
	uint16_t MatrixInCol = DMD_1COLOR_ScreenHeight / DMD_1COLOR_MatrixHeight;
	uint8_t MatrixLines8 = DMD_1COLOR_MatrixHeight >> 3;
	uint16_t StartIdxRow = 0, StartIdxCol = 0;

	for (uint8_t Row = 0; Row < MatrixInRow; Row++)
	{
		StartIdxCol = StartIdxRow;

		for (uint8_t Col = 0; Col < MatrixInCol; Col++)
		{
			// Копируем буфер кадра одной матрицы
			uint8_t *pDst = DMD_1COLOR_MatrixBuff;
			uint8_t *pSrc = &pBuff[StartIdxCol];

			for (uint8_t Line8 = 0; Line8 < MatrixLines8; Line8++)
			{
				memcpy(pDst, pSrc, DMD_1COLOR_MatrixWidth);
				DMD_1COLOR_SendFromMatrixBuff(Row4, DMD_1COLOR_MatrixBuff);
				pDst += DMD_1COLOR_MatrixWidth;
				pSrc += DMD_1COLOR_ScreenWidth;
			}

			StartIdxCol += DMD_1COLOR_MatrixWidth;
		}
		StartIdxRow += MatrixLines8 * DMD_1COLOR_ScreenWidth;
	}
}
//==============================================================================


//==============================================================================
// Процедура обновляет 1 матрицу
//==============================================================================
void DMD_1COLOR_SendFromMatrixBuff(uint8_t Row4, uint8_t *pBuff)
{
	uint8_t MatrixRows8bit = DMD_1COLOR_MatrixHeight >> 3;
	uint8_t MatrixCols8bit = DMD_1COLOR_MatrixWidth >> 3;
	uint8_t *pPart = (uint8_t *) &(DMD_1COLOR_Buff[Row4][0]);

	for (uint8_t Col = 0; Col < MatrixCols8bit; Col++)
	{
		for (uint8_t Row = MatrixRows8bit; Row; Row--)
		{
			uint8_t *p8Bytes = pBuff + ((Row - 1) * DMD_1COLOR_MatrixWidth);
			p8Bytes += (Col << 3);

			*(pPart++) = DMD_1COLOR_GetFrom8ScreenBytes(1 << (Row4 + 4), p8Bytes);
			*(pPart++) = DMD_1COLOR_GetFrom8ScreenBytes(1 << Row4, p8Bytes);
		}
	}
}
//==============================================================================



//==============================================================================
// Функция возвращает байт (строку из 8 светодиодов) по 8-байтному фрагменту буфера
// графической библиотеки и маске соответствующей строки светодиодов во фрагменте
//==============================================================================
uint8_t DMD_1COLOR_GetFrom8ScreenBytes(uint8_t Mask, uint8_t *pBuff)
{
	uint8_t Byte = 0;

	//Переводим "вертикальные" байты из массива в горизонтальные
	//в соответствии с текущей строкой.
	//В итоге получается массив, где биты расположены горизонтально,
	//Первый бит слева внизу, счет увеличивается вверх и вправо.
	//Но строки в секторе расположены сверху вниз
	for (uint8_t BitMask = 0x80; BitMask; BitMask >>= 1, pBuff++)
	{
		if (*pBuff & Mask)
			Byte |= BitMask;
	}

//#if (DMD_1COLOR_Inverse == 1)
	//Инвертируем данные (так как LED панель горит при 0)
	Byte ^= 0xFF;
//#endif

	return Byte;
}
//==============================================================================


//Копирование из подготовленного буфера в массивиы для построчного вводы в матрицу
void prepare_data() {
	//Идем по строкам
	for (uint8_t i = 0; i < sizeof(DMD_1COLOR_Buff) / sizeof(DMD_1COLOR_Buff[0]); i++) {
		//По столбцам
		for (uint8_t k = 0; k < sizeof(DMD_1COLOR_Buff[0]); k++) {
			if (i == 0) {
				data1[k] = DMD_1COLOR_Buff[i][k];
			}
			else if (i == 1) {
				data2[k] = DMD_1COLOR_Buff[i][k];
			}
			else if (i == 2) {
				data3[k] = DMD_1COLOR_Buff[i][k];
			}
			else if (i == 3) {
				data4[k] = DMD_1COLOR_Buff[i][k];
			}
		}
	}
}
*/
