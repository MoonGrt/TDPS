//#include <stdio.h>
//#include <stdint.h>
//#include <math.h>
//
//void splitFloat(float num, uint8_t *splitdata)
//{
//    uint8_t integerPart = (uint8_t)num; // 将浮点数强制转换为uint8_t类型，只保留整数部分
//    float decimal = num - integerPart;  // 获取小数部分
//    decimal = round(decimal * 100);     // 将小数部分乘以100，并四舍五入到整数
//    uint8_t decimalPart = (uint8_t)decimal;
//    splitdata[0] = integerPart;
//    splitdata[1] = decimalPart;
//}
//
//float combineFloat(uint8_t *splitdata)
//{
//    float result = (float)splitdata[0] + (float)splitdata[1] / 100.0; // 将整数部分和小数部分合并
//    return result;
//}
//
//int main()
//{
//    float inputNum = 97.98;
//    uint8_t splitdata[2];
//
//    splitFloat(inputNum, splitdata);
//
//    printf("integer: %d %c\n", splitdata[0], splitdata[0]);
//    printf("decimal: %d %c\n", splitdata[1], splitdata[1]);
//
//    printf("float: %f\n", combineFloat(splitdata));
//
//    return 0;
//}
