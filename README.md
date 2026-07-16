# TUYA ESP32-C6 Project

這是一個將 nRF52832 TuyaOS BLE 專案移植到 ESP32-C6 的 ESP-IDF 專案。

目前已完成 ESP-NimBLE Tuya BLE transport、FD50 GATT Service、Write/Notify、
Tuya 憑證 NVS 儲存介面及雙槽 OTA 分區。完整 Smart Life 配對仍需要 Tuya
提供 ESP32-C6/RISC-V 相容的 secure protocol library。詳細狀態請參考
[`MIGRATION_STATUS.md`](MIGRATION_STATUS.md)。

## 項目結構

```
TUYA_ESP32C6/
├── main/                    # 主應用程式
│   ├── main.c              # 主程式與 BLE transport 測試入口
│   └── CMakeLists.txt       # 主件構建配置
├── components/             # BLE transport、Tuya storage 與共用元件
├── build/                  # 構建輸出目錄（編譯後生成）
├── CMakeLists.txt          # 頂級構建配置
├── partitions.csv          # Flash 分區表
├── sdkconfig.defaults      # ESP-IDF 默認配置
├── idf_component.yml       # 元件清單
└── README.md               # 本文件
```

## 前置條件

1. **ESP-IDF**: 版本 5.1 或更高
   - 安裝指南: https://docs.espressif.com/projects/esp-idf/

2. **工具鏈**: ESP32-C6 GCC 工具鏈

3. **串列埠驅動**: USB 至 UART 驅動程式

## 快速開始

### 1. 設定環境

```bash
# 設定 IDF_PATH（Linux/macOS）
export IDF_PATH=~/esp/esp-idf
source $IDF_PATH/export.sh

# Windows (PowerShell)
$env:IDF_PATH = "C:\esp\esp-idf"
& "$env:IDF_PATH\tools\idf_tools_to_powershell.bat"
```

### 2. 配置項目

```bash
cd TUYA_ESP32C6
idf.py set-target esp32c6
idf.py menuconfig
```

`sdkconfig.defaults` 已啟用 ESP-NimBLE 並將最大連線數設定為 1。

### 3. 編譯

```bash
idf.py build
```

### 4. 燒錄

```bash
idf.py -p /dev/ttyUSB0 flash     # Linux/macOS
idf.py -p COM3 flash              # Windows
```

### 5. 監視序列埠輸出

```bash
idf.py -p /dev/ttyUSB0 monitor    # Linux/macOS
idf.py -p COM3 monitor             # Windows
```

### 整合命令

```bash
idf.py -p /dev/ttyUSB0 build flash monitor
```

## 配置說明

### sdkconfig.defaults

主要配置項：

- `CONFIG_IDF_TARGET="esp32c6"` - 目標芯片
- `CONFIG_BT_ENABLED=y` - 啟用藍牙
- `CONFIG_BT_NIMBLE_ENABLED=y` - 啟用 ESP-NimBLE
- `CONFIG_BT_NIMBLE_ATT_PREFERRED_MTU=247` - 支援 244-byte ATT payload
- `CONFIG_PARTITION_TABLE_CUSTOM=y` - 使用 OTA/Tuya NVS 分區表
- `CONFIG_SPI_FLASH_FREQ_80M=y` - Flash 時鐘

編輯 `sdkconfig.defaults` 或使用 `idf.py menuconfig` 修改配置。

## 開發指南

### 添加新元件

1. 在 `components/` 目錄下建立新資料夾
2. 添加 `CMakeLists.txt` 和源文件
3. 元件會自動被構建系統發現

### 使用協力廠商庫

在 `idf_component.yml` 中添加相依關係：

```yaml
dependencies:
  esp-idf:
    version: ">=5.1"
  # 添加其他依賴
```

## 常見問題

### 序列埠連接問題
- 確保 USB 驅動程式已安裝
- 檢查設備管理器中的 COM 埠
- 嘗試其他 USB 線或埠

### 編譯錯誤
- 清潔構建: `idf.py fullclean`
- 更新 ESP-IDF: `cd $IDF_PATH && git pull`

### 燒錄失敗
- 重置設備: 按住 BOOT 鍵
- 檢查波特率設定

## 參考資源

- [ESP-IDF 官方文檔](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-C6 資料表](https://www.espressif.com/en/products/socs/esp32-c6)
- [Tuya 文檔](https://developer.tuya.com/)

## 授權

MIT License - 詳見 LICENSE 文件

## 版本信息

- 專案版本: 1.0.0
- 目標芯片: ESP32-C6
- ESP-IDF 版本: 5.1+
- 建立日期: 2026-07-16
