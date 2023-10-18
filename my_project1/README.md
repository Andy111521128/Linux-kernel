# Linux Project1

## 目標
1. 實做一個 system call ，將 virtual address 轉為 physical address 。
2. 利用這個 system call 將 thread/process 每個 segment 的 memory location 找出來 ，並把 project address space layout 畫出來 。

>http://staff.csie.ncu.edu.tw/hsufh/COURSES/FALL2023/linux_project_1.html

## Kernel 與 OS 版本
1. Linux Kernel 5.8.1
2. Ubuntu 20.04 LTS desktop-amd
3. virtual box 7.0
   - 23 cores
   - 12G RAM

## Kernel 編譯/新增System call的過程所遇到的問題
我主要是參考以下教學步驟：

><https://hackmd.io/MBn8nRiLQFy66B0L_RFevg?view#Result>
><https://blog.kaibro.tw/2016/11/07/Linux-Kernel%E7%B7%A8%E8%AD%AF-Ubuntu/>
><https://dev.to/jasper/adding-a-system-call-to-the-linux-kernel-5-8-1-in-ubuntu-20-04-lts-2ga8>
