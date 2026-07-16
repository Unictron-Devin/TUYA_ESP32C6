# ESP32-C6 Tuya 項目設定指南

## 第一步：設定環境變數

### Windows

1. 打開 **環境變數設定**：
   - 按 `Win + X` → 選 **系統** → **進階系統設定** → **環境變數**
   - 或搜尋 "編輯環境變數"

2. 新增系統環境變數：
   - **變數名稱**: `IDF_PATH`
   - **變數值**: `C:\esp\esp-idf` (實際路徑)

3. 新增 `PATH` 環境變數：
   - 將 `C:\esp\esp-idf\tools` 加入 PATH

4. 測試設定：
   ```cmd
   echo %IDF_PATH%
   idf.py --version
   ```

### Linux / macOS

```bash
export IDF_PATH=$HOME/esp/esp-idf
source $IDF_PATH/export.sh
```

可將上述代碼加入 `~/.bashrc` 或 `~/.zshrc`

## 第二步：配置目標芯片

```bash
cd TUYA_ESP32C6
idf.py set-target esp32c6
```

## 第三步：調整項目配置

```bash
idf.py menuconfig
```

進入菜單後調整以下配置：

### WiFi 設定 (optional)
```
Component config → ESP Wi-Fi
  ├─ WiFi SSID: YOUR_SSID
  └─ WiFi Password: YOUR_PASSWORD
```

### 藍牙設定 (optional)
```
Component config → Bluetooth
  ├─ Bluetooth → Bluetooth Controller Mode: BLE Only
  ├─ Bluetooth LE → Max ACL Connections: 4
  └─ Bluetooth LE → Max Sync Connections: 0
```

### 其他常用設定
```
Serial flasher config → Flash SPI frequency: 80 MHz
Serial flasher config → Flash SPI mode: QIO
```

## 第四步：編譯項目

```bash
idf.py build
```

**預期輸出:**
```
...
Building project...
[100%] Built target ...
Build finished successfully.
Project build complete.
```

## 第五步：燒錄到設備

1. 連接 USB 線到 ESP32-C6
2. 檢查 COM 埠：
   - **Windows**: 設備管理器 → COM 埠
   - **Linux/macOS**: `ls /dev/tty*`

3. 燒錄：
   ```bash
   idf.py -p COM3 flash      # Windows (改成實際 COM 埠)
   idf.py -p /dev/ttyUSB0 flash  # Linux
   ```

## 第六步：監視序列埠輸出

```bash
idf.py -p COM3 monitor      # Windows
idf.py -p /dev/ttyUSB0 monitor  # Linux
```

**預期輸出:**
```
ESP-ROM:esp32c6-20220919
Build:Sep 19 2022
...
I (xxx) TUYA_ESP32C6: ESP32-C6 Tuya Application Started
I (xxx) TUYA_ESP32C6: This is esp32c6 chip with 1 CPU core(s), WiFi/BLE/802.15.4 (Zigbee/Thread)
I (xxx) TUYA_ESP32C6: silicon revision v0
I (xxx) TUYA_ESP32C6: Flash size: 4 MB
I (xxx) TUYA_ESP32C6: Application is running...
```

## 快速命令組合

```bash
# 編譯 + 燒錄 + 監視
idf.py -p COM3 build flash monitor

# 完全清潔構建
idf.py fullclean

# 查看構建資訊
idf.py size
idf.py size-components
```

## VS Code 集成

### 執行任務

在 VS Code 中按 `Ctrl+Shift+B` 進行構建，或 `Ctrl+Shift+P` 搜尋任務：

- **ESP32-C6 Build** - 編譯
- **ESP32-C6 Flash** - 燒錄
- **ESP32-C6 Monitor** - 監視
- **ESP32-C6 Full Build** - 編譯+燒錄+監視

### 調試設定

編輯 `.vscode/launch.json` 中的 `COM3` 為你的實際 COM 埠。

## 常見問題排除

### 1. 找不到 idf.py 命令
```
ERROR: idf.py not found
```
**解決**: 確保 `IDF_PATH` 已正確設定

### 2. 序列埠連接失敗
```
ERROR: Failed to connect to ESP32
```
**解決**:
- 檢查 USB 連接
- 嘗試按住 BOOT 鍵後再燒錄
- 更新 USB 驅動程式

### 3. 編譯錯誤
```
Error: component *** not found
```
**解決**:
```bash
idf.py fullclean
idf.py build
```

### 4. 波特率問題
```
ERROR: Timeout waiting for packet
```
**解決**: 在 menuconfig 中調整：
```
Serial flasher config → Baud rate: 921600 (或 460800)
```

## 下一步

1. **修改 main.c** - 在 `main/main.c` 中添加你的應用邏輯
2. **添加組件** - 在 `components/` 中建立新的可重用元件
3. **整合 Tuya SDK** - 添加 Tuya IoT SDK 依賴

## 有用資源

- [ESP-IDF 官方文檔](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-C6 資料表](https://www.espressif.com/en/products/socs/esp32c6/resources)
- [IDF 組件表](https://components.espressif.com/)
- [Tuya IoT 開發者中心](https://developer.tuya.com/)

---

**最後修改**: 2026-07-16  
**適用版本**: ESP-IDF 5.1+
