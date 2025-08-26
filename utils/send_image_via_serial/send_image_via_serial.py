#!/usr/bin/env python3

import serial
import time
from PIL import Image

def rgb888_to_rgb565(r, g, b):
    """RGB888转RGB565格式"""
    r5 = (r >> 3) & 0x1F  # 5位红色
    g6 = (g >> 2) & 0x3F  # 6位绿色
    b5 = (b >> 3) & 0x1F  # 5位蓝色
    return (r5 << 11) | (g6 << 5) | b5

def resize_image(image, target_width=240, target_height=320):
    """缩放图像到目标尺寸，保持宽高比并居中裁剪"""
    orig_width, orig_height = image.size
    
    # 计算缩放比例（选择较大的比例确保完全覆盖）
    scale = max(target_width / orig_width, target_height / orig_height)
    
    # 缩放图像
    new_width = int(orig_width * scale)
    new_height = int(orig_height * scale)
    image = image.resize((new_width, new_height), Image.Resampling.LANCZOS)
    
    # 居中裁剪到目标尺寸
    left = (new_width - target_width) // 2
    top = (new_height - target_height) // 2
    return image.crop((left, top, left + target_width, top + target_height))

def send_image(image_path, port='COM3', baudrate=115200):
    """
    发送图像到STM32显示器
    
    Args:
        image_path: 图像文件路径
        port: 串口号
        baudrate: 波特率
    """
    # 1. 加载并处理图像
    print(f"处理图像: {image_path}")
    image = Image.open(image_path).convert('RGB')
    image = resize_image(image, 240, 320)
    pixels = image.load()
    print(f"图像尺寸: {image.size}")
    
    # 2. 打开串口
    print(f"连接串口: {port}")
    ser = serial.Serial(port, baudrate, timeout=5)
    time.sleep(2)  # 等待连接稳定
    
    # 3. 等待STM32就绪
    print("等待STM32就绪...")
    while True:
        if ser.in_waiting > 0:
            response = ser.readline().decode('utf-8', errors='ignore').strip()
            print(f"STM32: {response}")
            if "READY" in response:
                break
        time.sleep(0.5)
    
    # 4. 分块传输图像 (每块4行，1920字节)
    print("开始传输...")
    rows_per_chunk = 4
    total_chunks = 320 // rows_per_chunk  # 80块
    
    for chunk_idx in range(total_chunks):
        start_row = chunk_idx * rows_per_chunk
        end_row = start_row + rows_per_chunk
        
        print(f"发送块 {chunk_idx+1}/{total_chunks}: 行{start_row}-{end_row-1}")
        
        # 等待STM32请求这一块
        while True:
            if ser.in_waiting > 0:
                response = ser.readline().decode('utf-8', errors='ignore').strip()
                if f"READY_CHUNK_{chunk_idx}" in response:
                    break
            time.sleep(0.1)
        
        # 准备数据块 (4行 × 240像素 × 2字节 = 1920字节)
        chunk_data = bytearray()
        for y in range(start_row, end_row):
            for x in range(240):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                # 高字节在前，低字节在后
                chunk_data.extend([(rgb565 >> 8) & 0xFF, rgb565 & 0xFF])
        
        # 发送数据
        ser.write(chunk_data)
        ser.flush()
        
        # 等待确认
        while True:
            if ser.in_waiting > 0:
                response = ser.readline().decode('utf-8', errors='ignore').strip()
                if f"CHUNK_{chunk_idx}_OK" in response:
                    break
            time.sleep(0.1)
        
        # 显示进度
        progress = (chunk_idx + 1) / total_chunks * 100
        print(f"  进度: {progress:.1f}%")
    
    print("传输完成!")
    ser.close()

def main():
    import sys
    
    if len(sys.argv) != 2:
        print("用法: python send_image_simple.py <图像文件>")
        print("示例: python send_image_simple.py test_pattern.png")
        sys.exit(1)
    
    image_path = sys.argv[1]
    
    try:
        send_image(image_path)
    except Exception as e:
        print(f"错误: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
