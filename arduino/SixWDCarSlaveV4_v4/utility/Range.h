/*******************************************************************************
 * File name: Range.h
 * Created on: 2018/06/13 23:40:52
 * Author: Wuxy
 * 范围条件宏
 * 
 * History:
 * 	v1
 * 		2018/06/13 创建文件
*******************************************************************************/
#ifndef RANGE_H_
#define RANGE_H_

#define In_Range(x, min, max) ((x) >= (min) && (x) <= (max))
#define In_Error_At_Ref(x, ref, error) In_Range(x, ref - error, ref + error)
#define In_Error(x, error) In_Error_At_Ref(x, 0, error)

#endif /* RANGE_H_ */
