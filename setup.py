#!/usr/bin/env python3
"""
MimoCH395 工程自动组装脚本
运行环境: Windows (Python 3.6+)
功能: 下载STM32CubeF1和FreeRTOS源码，组装完整Keil工程
"""
import os
import sys
import shutil
import zipfile
import tempfile

try:
    import urllib.request
except ImportError:
    print("ERROR: 需要Python 3.6+")
    sys.exit(1)

# ======================== 配置 ========================
PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
APP_DIR = os.path.join(PROJECT_DIR, "App")
CORE_DIR = os.path.join(PROJECT_DIR, "Core")

# STM32CubeF1 GitHub release
CUBE_URL = "https://github.com/STMicroelectronics/STM32CubeF1/archive/refs/heads/master.zip"
FREERTOS_URL = "https://github.com/FreeRTOS/FreeRTOS-Kernel/archive/refs/heads/main.zip"

# ======================== 下载函数 ========================
def download(url, dest):
    """下载文件，显示进度"""
    print(f"  下载: {url}")
    print(f"  目标: {dest}")
    urllib.request.urlretrieve(url, dest, reporthook=progress)
    print()

def progress(block, block_size, total_size):
    done = block * block_size
    if total_size > 0:
        pct = min(done * 100 // total_size, 100)
        mb = done / (1024*1024)
        total_mb = total_size / (1024*1024)
        print(f"\r  进度: {pct}% ({mb:.1f}/{total_mb:.1f} MB)", end="", flush=True)

# ======================== 主流程 ========================
def main():
    print("=" * 60)
    print("  MimoCH395 工程自动组装")
    print("=" * 60)

    tmp_dir = os.path.join(tempfile.gettempdir(), "mimo_ch395_deps")
    os.makedirs(tmp_dir, exist_ok=True)

    # ---- 1. 下载STM32CubeF1 ----
    cube_zip = os.path.join(tmp_dir, "stm32cubef1.zip")
    cube_dir = os.path.join(tmp_dir, "STM32CubeF1-master")

    if not os.path.exists(cube_dir):
        print("\n[1/4] 下载STM32CubeF1 HAL库...")
        download(CUBE_URL, cube_zip)
        print("  解压中...")
        with zipfile.ZipFile(cube_zip, 'r') as z:
            z.extractall(tmp_dir)
        os.remove(cube_zip)
    else:
        print("\n[1/4] STM32CubeF1 已缓存，跳过下载")

    # ---- 2. 下载FreeRTOS Kernel ----
    freertos_zip = os.path.join(tmp_dir, "freertos.zip")
    freertos_dir = os.path.join(tmp_dir, "FreeRTOS-Kernel-main")

    if not os.path.exists(freertos_dir):
        print("\n[2/4] 下载FreeRTOS Kernel...")
        download(FREERTOS_URL, freertos_zip)
        print("  解压中...")
        with zipfile.ZipFile(freertos_zip, 'r') as z:
            z.extractall(tmp_dir)
        os.remove(freertos_zip)
    else:
        print("\n[2/4] FreeRTOS Kernel 已缓存，跳过下载")

    # ---- 3. 复制HAL/CMSIS文件 ----
    print("\n[3/4] 组装HAL/CMSIS驱动...")

    # Drivers/HAL
    hal_src = os.path.join(cube_dir, "Drivers", "STM32F1xx_HAL_Driver")
    hal_dst = os.path.join(PROJECT_DIR, "Drivers", "STM32F1xx_HAL_Driver")
    if os.path.exists(hal_dst):
        shutil.rmtree(hal_dst)
    shutil.copytree(os.path.join(hal_src, "Inc"), os.path.join(hal_dst, "Inc"))
    shutil.copytree(os.path.join(hal_src, "Src"), os.path.join(hal_dst, "Src"))
    print(f"  HAL: {hal_dst}")

    # Drivers/CMSIS
    cmsis_src = os.path.join(cube_dir, "Drivers", "CMSIS")
    cmsis_dst = os.path.join(PROJECT_DIR, "Drivers", "CMSIS")
    if os.path.exists(cmsis_dst):
        shutil.rmtree(cmsis_dst)
    # 只复制需要的目录
    os.makedirs(os.path.join(cmsis_dst, "Include"), exist_ok=True)
    os.makedirs(os.path.join(cmsis_dst, "Device", "ST", "STM32F1xx", "Include"), exist_ok=True)
    os.makedirs(os.path.join(cmsis_dst, "Device", "ST", "STM32F1xx", "Source"), exist_ok=True)

    # CMSIS核心头文件
    cmsis_inc_src = os.path.join(cmsis_src, "Include")
    cmsis_inc_dst = os.path.join(cmsis_dst, "Include")
    for f in os.listdir(cmsis_inc_src):
        if f.endswith(".h"):
            shutil.copy2(os.path.join(cmsis_inc_src, f), os.path.join(cmsis_inc_dst, f))

    # 设备头文件
    dev_src = os.path.join(cmsis_src, "Device", "ST", "STM32F1xx", "Include")
    dev_dst = os.path.join(cmsis_dst, "Device", "ST", "STM32F1xx", "Include")
    for f in os.listdir(dev_src):
        if f.endswith(".h"):
            shutil.copy2(os.path.join(dev_src, f), os.path.join(dev_dst, f))

    # 启动文件 (startup)
    startup_src = os.path.join(cmsis_src, "Device", "ST", "STM32F1xx", "Source", "Templates", "arm")
    startup_dst = os.path.join(cmsis_dst, "Device", "ST", "STM32F1xx", "Source")
    if os.path.exists(startup_src):
        for f in os.listdir(startup_src):
            if f.endswith(".s") and "103xG" in f.upper():
                shutil.copy2(os.path.join(startup_src, f), os.path.join(startup_dst, f))
                print(f"  启动文件: {f}")
    else:
        # 尝试GCC目录
        startup_src_gcc = os.path.join(cmsis_src, "Device", "ST", "STM32F1xx", "Source", "Templates", "gcc")
        if os.path.exists(startup_src_gcc):
            for f in os.listdir(startup_src_gcc):
                if f.endswith(".s") and "103xG" in f.upper():
                    shutil.copy2(os.path.join(startup_src_gcc, f), os.path.join(startup_dst, f))
                    print(f"  启动文件: {f}")

    print(f"  CMSIS: {cmsis_dst}")

    # ---- 4. 复制FreeRTOS文件 ----
    print("\n[4/4] 组装FreeRTOS...")

    fr_src = freertos_dir
    fr_dst = os.path.join(PROJECT_DIR, "Middlewares", "Third_Party", "FreeRTOS", "Source")
    if os.path.exists(fr_dst):
        shutil.rmtree(fr_dst)
    os.makedirs(fr_dst, exist_ok=True)

    # 复制核心源码
    for item in ["include", "tasks.c", "queue.c", "list.c", "timers.c",
                  "event_groups.c", "stream_buffer.c", "croutine.c"]:
        src = os.path.join(fr_src, item)
        dst = os.path.join(fr_dst, item)
        if os.path.isfile(src):
            shutil.copy2(src, dst)
        elif os.path.isdir(src):
            shutil.copytree(src, dst)

    # portable层
    port_src = os.path.join(fr_src, "portable")
    port_dst = os.path.join(fr_dst, "portable")
    os.makedirs(port_dst, exist_ok=True)

    # GCC/ARM_CM3 port
    gcc_port_src = os.path.join(port_src, "GCC", "ARM_CM3")
    gcc_port_dst = os.path.join(port_dst, "GCC", "ARM_CM3")
    if os.path.exists(gcc_port_src):
        if os.path.exists(gcc_port_dst):
            shutil.rmtree(gcc_port_dst)
        shutil.copytree(gcc_port_src, gcc_port_dst)

    # Keil/ARM_CM3 port (用于Keil编译)
    keil_port_src = os.path.join(port_src, "RVDS", "ARM_CM3")
    keil_port_dst = os.path.join(port_dst, "RVDS", "ARM_CM3")
    if os.path.exists(keil_port_src):
        if os.path.exists(keil_port_dst):
            shutil.rmtree(keil_port_dst)
        shutil.copytree(keil_port_src, keil_port_dst)

    # heap_4.c
    heap_src = os.path.join(port_src, "MemMang")
    heap_dst = os.path.join(port_dst, "MemMang")
    if os.path.exists(heap_src):
        if os.path.exists(heap_dst):
            shutil.rmtree(heap_dst)
        shutil.copytree(heap_src, heap_dst)

    # CMSIS_RTOS封装
    cmsis_rtos_src = os.path.join(fr_src, "CMSIS_RTOS")
    cmsis_rtos_dst = os.path.join(fr_dst, "CMSIS_RTOS")
    if os.path.exists(cmsis_rtos_src):
        if os.path.exists(cmsis_rtos_dst):
            shutil.rmtree(cmsis_rtos_dst)
        shutil.copytree(cmsis_rtos_src, cmsis_rtos_dst)
    elif os.path.exists(os.path.join(fr_src, "CMSIS_RTOS_V2")):
        cmsis_rtos_v2_src = os.path.join(fr_src, "CMSIS_RTOS_V2")
        if os.path.exists(cmsis_rtos_dst):
            shutil.rmtree(cmsis_rtos_dst)
        shutil.copytree(cmsis_rtos_v2_src, cmsis_rtos_dst)

    print(f"  FreeRTOS: {fr_dst}")

    # ---- 完成 ----
    print("\n" + "=" * 60)
    print("  ✅ 工程组装完成!")
    print("=" * 60)
    print(f"\n项目目录: {PROJECT_DIR}")
    print("\n下一步:")
    print("  1. 用Keil打开 MimoCH395.uvprojx")
    print("  2. 或用CubeMX打开 MimoCH395.ioc 生成Keil工程")
    print("  3. 编译烧录")
    print()

if __name__ == "__main__":
    main()
