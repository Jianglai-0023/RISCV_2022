# RISCV Simulator
## Tomasulo架构图
![Image text](https://github.com/Jianglai-0023/RISCV_2022/blob/main/IMG_1630.jpg)
## 分支预测
### 预测跳转&&二级饱和分支预测
| 数据点| 预测跳转命中率 | 二级饱和预测命中率 |
| :----:| :----: | :----: |
| array_test1 | 0.45 | 0.5 |
| array_test2| 0.5 | 0.46 |
| basicopt1| 0.41 | 0.79 |
| bulgarian| 0.49 | 0.93 |
| expr| 0.62| 0.83 |
| gcd| 0.38 | 0.63 |
| hanoi| 0.5 | 0.61 |
|lvalue2| 0.33 | 0.67 |
|magic| 0.47| 0.72 |
|manyarguments| 0.2 | 0.8 |
|multiarray| 0.81 | 0.73 |
|naive| /| / |
|pi| 0.58 | 0.79 |
|qsort| 0.70 | 0.87 |
|queens| 0.37 | 0.71 |
|statement_test| 0.40 | 0.61 |
|superloop| 0.13 | 0.94 |
|tak| 0.25 | 0.77 |
### 组合逻辑
```
run_rob();  
run_slbuffer();  
run_reservation();  
```
### 时序逻辑
```
update();
run_issue();
```




