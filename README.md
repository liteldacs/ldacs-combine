# Ldacs-Combine

本项目用于测试对接接口的符合性

[![GPLv3 License](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

---

## 依赖

安装libldcauc：`https://github.com/liteldacs/libldcauc.git`

## 安装
```shell
git clone https://github.com/liteldacs/libldcauc.git
cd libldcauc && mkdir build && cd build
cmake ..  
make -j12 && sudo make install
```

## 运行

### **项目运行选项**

可通过命令行参数指定运行模式，各选项功能如下：

| 选项 | 短参数  | 功能描述                   | 示例命令                 | 备注        |
|----|------|------------------------|-----------------------------|-----------|
| c  | `-c` | 配置文件路径                 | `ldacs-combine -c xxx.yaml` |           |
| H  | `-H` | 启用Http模式，否则为terminal模式 | `ldacs-combine -H`          | 无需关注      |
| M  | `-M` | 启用Merge模式              | `ldacs-combine -M`          | 无需关注      |

- AS角色测试
```shell
ldacs-combine -c /usr/local/ldacs-cauc/config/ldacs_config_as_1.yaml
```
- GS角色测试
```shell
ldacs-combine -c /usr/local/ldacs-cauc/config/ldacs_config_gs.yaml
```
- AS角色测试
```shell
ldacs-combine -c /usr/local/ldacs-cauc/config/ldacs_config_sgw.yaml
```
## 版本
- **最后更新**: 2025/4/18

---

## 作者

中国民航大学新航行系统研究所


